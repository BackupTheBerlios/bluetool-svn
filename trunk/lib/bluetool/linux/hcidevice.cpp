#include "hcidevice_p.h"

namespace Hci
{
#if 0
LocalDevices LocalDevice::enumerate()
{
	LocalDevices ret;
	Socket sock;

	struct
	{
		u16 ndevs;
		hci_dev_req devs[HCI_MAX_DEV];
	} list_req;

	list_req.ndevs = HCI_MAX_DEV;

	if (!ioctl(sock.handle(), HCIGETDEVLIST, (void *) &list_req))
	{
		for( int i = 0; i < list_req.ndevs; ++i )
		{
			ret.push_back( LocalDevice( list_req.devs[i].dev_id ) );
		}
	}
	return ret;
}
#endif

/*	device control
*/

void LocalDevice::up()
{
	hci_dbg_enter();

	ulong i = id();

	Socket sock;
	if( ioctl(sock.handle(), HCIDEVUP, i) < 0 && errno != EALREADY )
		throw Exception();

	hci_dbg_leave();
}

void LocalDevice::on_up()
{
	hci_dbg_enter();

	/*hci_dev_info di;
	do
	{
		if(hci_devinfo(id(), &di) < 0)
			throw Exception();
	}
	while(!hci_test_bit(HCI_UP, &di.flags));*/
	//no, I don't like this, but I'm in a fucking hurry

	usleep(1000000);

	pvt->dd.renew(); // creates a new control socket
	pvt->init();

	hci_dbg_leave();
}

void LocalDevice::down()
{
	hci_dbg_enter();

	ulong i = id();

	Socket sock;
	if( ioctl(sock.handle(), HCIDEVDOWN, i) < 0 )
		throw Exception();

	hci_dbg_leave();
}

void LocalDevice::on_down()
{
	hci_dbg_enter();

	FdNotifier::destroy(pvt->notifier);
	pvt->notifier = NULL;

	pvt->dd.close();

	pvt->flush_queues();

	hci_dbg_leave();
}

void LocalDevice::reset()
{
	up();
	down();
}

/*
*/

Request::Request()
{
	to = Timeout::create();

	memset(&hr, 0, sizeof(hr));
	memset(iobuf, 0, sizeof(iobuf));
	memset(&ch, 0, sizeof(ch));
}

Request::~Request()
{
	/*	TODO: I think this leaks memory
	*/
	if(hr.cparam) delete[] (u8*)hr.cparam;
	if(hr.rparam) delete[] (u8*)hr.rparam;

	Timeout::destroy(to);
}

/*
*/

LocalDevice::Private::Private( LocalDevice* p, int i )
:	notifier(NULL)
{
	parent = p;
	id = i;
}

LocalDevice::Private::~Private()
{
/*	Requests::iterator rqi = dispatchq.begin();
	while(rqi != dispatchq.end())
	{
		delete *rqi;
		++rqi;
	}
	rqi = waitq.begin();
	while(rqi != waitq.end())
	{
		delete *rqi;
		++rqi;
	}
*/
	RemoteDevPTable::iterator rdi = inquiry_cache.begin();
	while(rdi != inquiry_cache.end())
	{
		delete rdi->second;
		++rdi;
	}

	FdNotifier::destroy(notifier);
}

void LocalDevice::Private::init()
{
	hci_dbg_enter();

	if( dd.handle() < 0 || !dd.bind(id) || hci_devba(id,(bdaddr_t*)&ba) < 0 )
		throw Exception();

	//delete notifier; //not needed, on_down frees it already
	notifier = FdNotifier::create(dd.handle(), POLLIN);
	notifier->can_read.connect(sigc::mem_fun( this, &LocalDevice::Private::read_ready ));
	notifier->can_write.connect(sigc::mem_fun( this, &LocalDevice::Private::write_ready ));

	Filter f;
	f.set_type(HCI_EVENT_PKT);
	f.set_event(EVT_CMD_STATUS);
	f.set_event(EVT_CMD_COMPLETE);
	f.set_event(EVT_INQUIRY_RESULT);
	f.clear_opcode();

	if(!dd.set_filter(f))
		throw Exception();

	/* when the this device object is created
	   we update the connections/remote tables
	   with connections existing before the daemon
	   was launched
	*/

	hci_conn_list_req *cl;

	char buf[sizeof(hci_conn_list_req) + sizeof(hci_conn_info) * 10];

	cl = (hci_conn_list_req*) buf;

	cl->dev_id = id;
	cl->conn_num = 10;

	if(ioctl(dd.handle(), HCIGETCONNLIST, buf))
	{
		hci_dbg_leave();
		throw Exception();
	}

	hci_conn_info* hi = cl->conn_info;

	for( int i = 0; i < cl->conn_num; ++i, ++hi )
	{
		char straddr[18] = {0};
		ba2str(&(hi->bdaddr),straddr);

		RemoteDevPTable::iterator riter = inquiry_cache.find(straddr);
		RemoteDevice* rptr;

		if( riter == inquiry_cache.end() )
		{
			/* save the remote device
			*/
			RemoteInfo ri;

			memset(&ri, 0, sizeof(ri));
			ri.addr = BdAddr(hi->bdaddr.b);
			ri.pscan_rpt_mode = 0x02;
				//default value taken from the BlueZ libs

			rptr = parent->on_new_cache_entry(ri);
			inquiry_cache[straddr] = rptr;
			
		}
		else
		{
			rptr = riter->second;
		}
		/* now add the connection to the list
		*/

		ConnPTable::iterator citer = rptr->_connections.find(hi->handle);

		if( citer == rptr->_connections.end() )
		{
			/* add connection
			*/
			ConnInfo ci;
			ci.handle = hi->handle;
			ci.link_type = hi->type;
			ci.encrypt_mode = 0;

			Connection* cptr = rptr->on_new_connection(ci);
			if(cptr)
				rptr->_connections[ci.handle] = cptr;
		}
	}

	hci_dbg_leave();	
}

void LocalDevice::Private::update_cache( RemoteInfo& info )
{
	const std::string straddr = info.addr.to_string();

	RemoteDevPTable::iterator i = inquiry_cache.find(straddr);
	if( i == inquiry_cache.end() )
	{
		/* create new entry
		*/
		RemoteDevice* rd = parent->on_new_cache_entry(info);

		if(rd)	inquiry_cache[straddr] = rd;
	}
	else
	{
		/* update cache entry
		*/
		i->second->update(info);
	}
}

void LocalDevice::Private::finalize_cache()
{
	RemoteDevPTable::iterator ri = inquiry_cache.begin();
	while( ri != inquiry_cache.end() )
	{
		if( TVLESS(ri->second->last_updated(), time_last_inquiry) )
		{
			delete ri->second;

			RemoteDevPTable::iterator tmp = ri;
			tmp++;
			inquiry_cache.erase(ri);
			ri = tmp;
		}
		else
		{
			++ri;
		}
	}
}

void LocalDevice::Private::clear_cache()
{
	RemoteDevPTable::iterator ri = inquiry_cache.begin();
	while( ri != inquiry_cache.end() )
	{
		RemoteDevPTable::iterator ti = ri;
		ti++;
	
		delete ri->second;
		inquiry_cache.erase(ri);
		ri = ti;
	}
}

void LocalDevice::Private::post_req( RefPtr<Request>& req )
{
	if(!req->hr.event) req->hr.event = EVT_CMD_COMPLETE;

	req->type = HCI_COMMAND_PKT;
	req->ch.opcode = htobs(cmd_opcode_pack( req->hr.ogf, req->hr.ocf ));
	req->ch.plen = req->hr.clen;
	req->iobuf[0].iov_base	= &req->type;
	req->iobuf[0].iov_len	= 1;
	req->iobuf[1].iov_base	= &req->ch;
	req->iobuf[1].iov_len	= HCI_COMMAND_HDR_SIZE;
	if( req->hr.clen )
	{
		req->iobuf[2].iov_base	= req->hr.cparam;
		req->iobuf[2].iov_len	= req->hr.clen;
		req->ion = 3;
	}
	else req->ion = 2;

	dispatchq.push_front(req);
	req->iter = dispatchq.begin();

	req->status = Request::QUEUED;
	req->to->data( &req->iter );
	req->to->timed_out.connect( sigc::mem_fun( this, &Private::req_timedout ));
	req->to->start();

	if(!notifier) //device not available at the moment
	{
		/*	I used to do this BEFORE pushing the current
			request into the queue, which resulted, of
			course in having it time out
		*/
		flush_queues();
		return;
	}

	notifier->flags( notifier->flags() | POLLOUT );
}

void LocalDevice::Private::req_timedout( Timeout& t )
{
	hci_dbg_enter();

	RefPtr<Request>& r =
		*(*( static_cast<std::list< RefPtr<Request> >::iterator*>(t.data()) ));
	// unreadable, but works

	r->status = Request::TIMEDOUT;
	r->to->stop();
	fire_event(r); //todo: fails if timeout occurs in dispatch queue

	hci_dbg_leave();
}


void LocalDevice::Private::flush_queues()
{
	hci_dbg_enter();

	/*	we're toast! cancel all pending I/O
	*/
	Requests::iterator d = dispatchq.begin();
	while(d != dispatchq.end())
	{
		waitq.push_back(*d);
		++d;
	}
	dispatchq.clear();

	Requests::iterator i = waitq.begin();

	while(i != waitq.end())
	{
		Requests::iterator n = i;
		++n;

		parent->on_after_event(EIO,(*i)->cookie);
		waitq.erase(i);

		i = n;
	}
	hci_dbg_leave();
}

void LocalDevice::Private::read_ready( FdNotifier& fn )
{
	hci_dbg_enter();

	/* ripped from file hci.c in Bluez 2.18
	*/
	u8 buf[HCI_MAX_EVENT_SIZE];
	int len;

	if(fn.state() & POLLERR || fn.state() & POLLHUP )
	{
		flush_queues();
		//throw Exception();
		hci_dbg_leave();
		return;
	}
	if(fn.flags() & POLLOUT)
	{
		/* that's NONSENSE but sometimes it BLOCKS if
		   we let him reach the read()
		*/

		hci_dbg("fn.flags() = %d",fn.flags());
		hci_dbg_leave();
		return;
	}

	while( (len = read(fn.fd(), buf, sizeof(buf))) < 0 )
	{
        	if (errno == EAGAIN || errno == EINTR)
			continue;

		flush_queues();
		throw Exception();
	}

	struct __hp
	{
		u8 type;
		hci_event_hdr eh;
		union
		{
			evt_cmd_status   cs;
			evt_cmd_complete cc;
			char ptr;
		} evt;
	} __PACKED;

	__hp* hp = (__hp*)buf;


	if(hp->type != HCI_EVENT_PKT)
	{
		hci_dbg_leave();
		return;
	}
	len -= (1 + HCI_EVENT_HDR_SIZE);

	Requests::reverse_iterator ri = waitq.rbegin();
	
	RefPtr<Request>pr;

	while( ri != waitq.rend() )
	{
		pr = *ri;
		if( pr->status == Request::WAITING )
		{
			switch (hp->eh.evt) {

			case EVT_CMD_STATUS:
			{
				/* we shouldn't deal with particular events here, but
				   there are some exceptions :(
				*/
				if( hp->evt.cs.opcode == cmd_opcode_pack(OGF_LINK_CTL,OCF_INQUIRY) 
					&& !hp->evt.cs.status )
				{
					gettimeofday(&time_last_inquiry, NULL);
				}

				if (hp->evt.cs.opcode != pr->ch.opcode)
					break;

				if (hp->evt.cs.status)
				{
					pr->hr.event = EVT_CMD_STATUS;
					pr->hr.rlen = EVT_CMD_STATUS_SIZE;
					pr->hr.rparam = malloc(pr->hr.rlen);
					memcpy(&(hp->evt.cs),pr->hr.rparam,pr->hr.rlen);
					fire_event(pr);
				}
				//TODO: use cs.ncmd for better queueing
				break;
			}
			case EVT_CMD_COMPLETE:
			{
				if (hp->evt.cc.opcode != pr->ch.opcode)
					break;

				char* ptr = &hp->evt.ptr + EVT_CMD_COMPLETE_SIZE;
				len -= EVT_CMD_COMPLETE_SIZE;

				pr->hr.rlen = MIN(len, pr->hr.rlen);
				pr->hr.rparam = malloc(pr->hr.rlen);
				memcpy(pr->hr.rparam, ptr, pr->hr.rlen);
				goto _fire;
			}
			/* special events which are caught even if there's no
			   associated request 
			*/
			case EVT_INQUIRY_RESULT:
			{
				u8* nrsp = (u8*)&(hp->evt.ptr);
				inquiry_info* ii = (inquiry_info*)(nrsp+1);

				hci_event_inquiry_result(*nrsp, ii);
				break;
			}
			case EVT_CONN_COMPLETE:
			{
				hci_event_conn_complete((evt_conn_complete*)&(hp->evt.ptr));
				break;
			}
			case EVT_DISCONN_COMPLETE:
			{
				hci_event_disconn_complete((evt_disconn_complete*)&(hp->evt.ptr));
				break;
			}
			default:
			{
				if (hp->eh.evt != pr->hr.event)
					break;

				pr->hr.rlen = MIN(len, pr->hr.rlen);
				pr->hr.rparam = malloc(pr->hr.rlen);
				memcpy(pr->hr.rparam, &hp->evt.ptr, pr->hr.rlen);
				goto _fire;
			}

			}
		}
		++ri;
	}
	hci_dbg_leave();
	return;
_fire:	pr->status = Request::COMPLETE;
	fire_event(pr);

	/* after this request is complete, go serve any
	   other which may still be pending in the queue
	*/
	notifier->flags( notifier->flags() | POLLOUT );

	hci_dbg_leave();
}

void LocalDevice::Private::write_ready( FdNotifier& fn )
{
	hci_dbg_enter();

	if(dispatchq.empty())
	{
		notifier->flags( notifier->flags() & ~POLLOUT );
		hci_dbg_leave();
		return;
	}

	Filter of;
	dd.get_filter(of);

	Requests::reverse_iterator ri = dispatchq.rbegin();
	RefPtr<Request> pr;
	while( ri != dispatchq.rend() )
	{
		pr = *ri;
		switch( pr->status )
		{
			case Request::WRITING:
			{
				goto _writev;
			}
			case Request::QUEUED:
			{
				if( pr->hr.event == EVT_CMD_STATUS || pr->hr.event == EVT_CMD_COMPLETE )
				{
					if(of.opcode())
					{
						notifier->flags( notifier->flags() & ~POLLOUT );
						break;
					}
				}
				else if(of.test_event(pr->hr.event))
				{
					notifier->flags( notifier->flags() & ~POLLOUT );
					break;
				}
				pr->status = Request::WRITING;
				goto _write;
			}
			case Request::TIMEDOUT:
			{	//move! timeouts are supposed to be in the other queue
				pr->status = Request::WAITING;
				waitq.push_front(pr);
				dispatchq.erase( pr->iter );
				pr->iter = waitq.begin();

				fire_event(pr);
				break;

			}
			case Request::WAITING:
			case Request::COMPLETE:
			;	//do nothing
		}
		++ri;
	}
	return;

_write:	if(	pr->hr.event == EVT_CMD_STATUS || 
		pr->hr.event == EVT_CMD_COMPLETE	
	)
		of.set_opcode( htobs(cmd_opcode_pack(pr->hr.ogf, pr->hr.ocf)) );
	else
		of.set_event( pr->hr.event );

	dd.set_filter(of);

_writev:if( writev(dd.handle(), pr->iobuf, pr->ion) < 0 )
	{
		if (errno == EAGAIN || errno == EINTR)
		{
			hci_dbg_leave();
			return;
		}
		else
		{
			flush_queues();
			//throw Exception();
			return;
		}
	}
	else
	{
		pr->status = Request::WAITING;
		dispatchq.erase( pr->iter );
		waitq.push_front(pr);
		pr->iter = waitq.begin();
	}

	hci_dbg_leave();
}

void LocalDevice::Private::fire_event( RefPtr<Request>& req )
{
	hci_dbg_enter();

	Filter of;

	//req->dest_type = Request::LOCAL; //this is the default
	//req->dest.loc = parent;

	int error = 0;

	if(req->status == Request::TIMEDOUT)
	{
		error = ETIMEDOUT;
	}
	else
	switch( req->hr.event )
	{
		case EVT_CMD_COMPLETE:
		{
			switch( req->hr.ogf )
			{
				case OGF_LINK_CTL:
				{
					link_ctl_cmd_complete(*req);
					break;
				}
				case OGF_LINK_POLICY:
				{
					link_policy_cmd_complete(*req);
					break;
				}
				case OGF_HOST_CTL:
				{
					host_ctl_cmd_complete(*req);
					break;
				}
				case OGF_INFO_PARAM:
				{
					info_param_cmd_complete(*req);
					break;
				}
				case OGF_STATUS_PARAM:
				{
					status_param_cmd_complete(*req);
					break;
				}
			};						
			break;
		}
		case EVT_CMD_STATUS:
		{
			evt_cmd_status* cs = (evt_cmd_status*) req->hr.rparam;
			if(!cs->status)
			{
				goto nodel;
			}

			error = bt_error(cs->status);

			//parent->on_status_failed(status, req->cookie);
			//goto after;
			break;
		}
		/*	command-specific events
		*/
		default:
		{
			hci_event_received(*req);
		}
	}

	dd.get_filter(of);

	if(	req->hr.event == EVT_CMD_STATUS || 
		req->hr.event == EVT_CMD_COMPLETE	
	)
		of.clear_opcode();
	else
		of.clear_event( req->hr.event );

	dd.set_filter(of);

/*	switch( req->dest_type )
	{
		case Request::LOCAL:
		req->dest.loc->on_after_event(req->cookie);
		break;

		case Request::REMOTE:
		req->dest.rem->on_after_event(req->cookie);
		break;

		case Request::CONNECTION:
		//req->dest.con->on_after_event(req->cookie);
		break;
	}
*/
	parent->on_after_event(error,req->cookie);
	waitq.erase( req->iter );

nodel:	hci_dbg_leave();
	return;
}

void LocalDevice::Private::link_ctl_cmd_complete( Request& req )
{
	hci_dbg_enter();

	switch( req.hr.ocf )
	{
		case OCF_INQUIRY_CANCEL:
		{
			/* if there are outstanding inquiries, remove them
			   and then send response
			*/
			Requests::reverse_iterator ri = waitq.rbegin();
			while( ri != waitq.rend() )
			{
				if((*ri)->hr.event == EVT_INQUIRY_COMPLETE)
					fire_event(*ri);
				++ri;
			}
			parent->on_inquiry_cancel(0,req.cookie);
			break;
		}
		case OCF_PERIODIC_INQUIRY:
		{
			parent->on_periodic_inquiry_started(0,req.cookie);
			break;
		}
		case OCF_EXIT_PERIODIC_INQUIRY:
		{
			parent->on_periodic_inquiry_cancel(0,req.cookie);
			break;
		}
	}

	hci_dbg_leave();
}

void LocalDevice::Private::link_policy_cmd_complete( Request& req )
{
}

void LocalDevice::Private::host_ctl_cmd_complete( Request& req )
{
	hci_dbg_enter();

	switch( req.hr.ocf )
	{
		case OCF_READ_AUTH_ENABLE:
		{
			u8* enable = (u8*) req.hr.rparam;

			parent->on_get_auth_enable(0, req.cookie, *enable);
			break;
		}
		case OCF_READ_ENCRYPT_MODE:
		{
			u8* mode = (u8*) req.hr.rparam;

			parent->on_get_encrypt_mode(0, req.cookie, *mode);
			break;
		}
		case OCF_READ_SCAN_ENABLE:
		{
			read_scan_enable_rp* r = (read_scan_enable_rp*) req.hr.rparam;

			parent->on_get_scan_enable(r->status,req.cookie,r->enable);
			break;
		}
		case OCF_READ_LOCAL_NAME:
		{
			read_local_name_rp* r = (read_local_name_rp*) req.hr.rparam;

			parent->on_get_name(r->status,req.cookie,(char*)r->name);
			break;
		}
		case OCF_READ_CLASS_OF_DEV:
		{
			read_class_of_dev_rp* r = (read_class_of_dev_rp*) req.hr.rparam;

			parent->on_get_class(r->status,req.cookie,r->dev_class);
			break;
		}
		case OCF_READ_VOICE_SETTING:
		{
			read_voice_setting_rp* r = (read_voice_setting_rp*) req.hr.rparam;

			parent->on_get_voice_setting(r->status,req.cookie,r->voice_setting);
			break;
		}
		case OCF_WRITE_AUTH_ENABLE:
		{
			parent->on_set_auth_enable(0,req.cookie);
			break;
		}
		case OCF_WRITE_ENCRYPT_MODE:
		{
			parent->on_set_encrypt_mode(0,req.cookie);
			break;
		}
		case OCF_WRITE_SCAN_ENABLE:
		{
			parent->on_set_scan_enable(0,req.cookie);
			break;
		}
		case OCF_CHANGE_LOCAL_NAME:
		{
			parent->on_set_name(0,req.cookie);
			break;
		}
	}
	hci_dbg_leave();
}

void LocalDevice::Private::info_param_cmd_complete( Request& req )
{
	hci_dbg_enter();

	switch( req.hr.ocf )
	{
		case OCF_READ_BD_ADDR:
		{
			read_bd_addr_rp* r = (read_bd_addr_rp*) req.hr.rparam;

			char local_addr[18] = {0};
			ba2str(&(r->bdaddr),local_addr);

			parent->on_get_address(r->status,req.cookie,local_addr);
			break;
		}
		case OCF_READ_LOCAL_VERSION:
		{
			read_local_version_rp* r = (read_local_version_rp*) req.hr.rparam;

			const char* hci_ver = hci_vertostr(r->hci_ver);
			const char* lmp_ver = lmp_vertostr(r->lmp_ver);
			const char* comp_str = bt_compidtostr(r->manufacturer);

			parent->on_get_version
			(
				r->status,
				req.cookie,
				hci_ver,
				r->hci_rev,
				lmp_ver,
				r->lmp_subver,
				comp_str
			);
			break;
		}
		case OCF_READ_LOCAL_FEATURES:
		{
			read_local_features_rp* r = (read_local_features_rp*) req.hr.rparam;

			char* lmp_feat = lmp_featurestostr(r->features," ",0);

			parent->on_get_features(r->status,req.cookie,lmp_feat);
			 //todo: format this field as a string array

			free(lmp_feat);
			break;
		}
	}
	hci_dbg_leave();
}

void LocalDevice::Private::status_param_cmd_complete( Request& req )
{
}

void LocalDevice::Private::hci_event_received( Request& req )
{
	hci_dbg_enter();

	switch(req.hr.event)
	{
		case EVT_INQUIRY_COMPLETE:
		{
			finalize_cache();

			//u16* status = (u16*) req.hr.rparam;

			parent->on_inquiry_complete(0, req.cookie);
			break;
		}
		/*	remote commands
		*/
		case EVT_REMOTE_NAME_REQ_COMPLETE:
		{
			evt_remote_name_req_complete* r = (evt_remote_name_req_complete*) req.hr.rparam;

			char straddr[18] = {0};
			ba2str(&(r->bdaddr),straddr);

			RemoteDevPTable::iterator i = inquiry_cache.find(straddr);

			if( i != inquiry_cache.end() )
			{
				i->second->on_get_name(r->status, req.cookie, (char*)r->name);
			}
			else
			{
				hci_dbg("remote name %s from non cached device %s!",r->name, straddr);
			}
			break;
		}		
		case EVT_READ_REMOTE_VERSION_COMPLETE:
		{
			evt_read_remote_version_complete* r = (evt_read_remote_version_complete*) req.hr.rparam;

			const char* lmp_ver = lmp_vertostr(r->lmp_ver);
			const char* comp_str = bt_compidtostr(r->manufacturer);

			req.src.remote->on_get_version
			(
				r->status,
				req.cookie,
				lmp_ver,
				r->lmp_subver,
				comp_str
			);
			break;
		}
		case EVT_READ_REMOTE_FEATURES_COMPLETE:
		{
			evt_read_remote_features_complete* r = (evt_read_remote_features_complete*) req.hr.rparam;

			char* lmp_feat = lmp_featurestostr(r->features," ",0);
			req.src.remote->on_get_features(r->status, req.cookie, lmp_feat);
			break;
		}
		case EVT_READ_CLOCK_OFFSET_COMPLETE:
		{
			evt_read_clock_offset_complete* r = (evt_read_clock_offset_complete*) req.hr.rparam;

			req.src.remote->on_get_clock_offset(r->status, req.cookie, r->clock_offset);
			break;
		}
		/*	connection
		*/
		case EVT_CONN_COMPLETE:
		{
			evt_conn_complete* r = (evt_conn_complete*) req.hr.rparam;

			if(r->status)
			{
				char straddr[18] = {0};

				ba2str(&(r->bdaddr),straddr);

				RemoteDevPTable::iterator i = inquiry_cache.find(straddr);

				if( i != inquiry_cache.end() )
				{
					ConnInfo ci =
					{
						r->handle,
						r->link_type,
						r->encr_mode
					};
					Connection* c = i->second->on_new_connection(ci);

					if(c) i->second->_connections[r->handle] = c;
				}
				else
				{
					hci_dbg("connection from non cached device %s!",straddr);
				}
			}
//			req.dest_type = Request::REMOTE;
//			req.dest.rem = i->second;
			break;
		}
		case EVT_DISCONN_COMPLETE:
		{
			evt_disconn_complete* r = (evt_disconn_complete*) req.hr.rparam;

			/* find connection object and delete it
			*/
			RemoteDevPTable::iterator ri = inquiry_cache.begin();
			while(ri != inquiry_cache.end())
			{
				if(ri->second->remove_connection(r->handle))
					break;
				++ri;
			}
			break;
		}
	}

	hci_dbg_leave();
}

void LocalDevice::Private::hci_event_inquiry_result( u8 nrsp, inquiry_info* ii )
{
	for( u8 i = 0; i < nrsp; ++i )
	{
		inquiry_info* iii = ii+i;

		BdAddr addr (iii->bdaddr.b);

		RemoteInfo info =
		{
			addr,
			{
				iii->dev_class[0],
				iii->dev_class[1],
				iii->dev_class[2],
			},
			iii->pscan_rep_mode,
			iii->pscan_period_mode,
			iii->pscan_mode,
			iii->clock_offset
		};
		update_cache(info);
	}
}

void LocalDevice::Private::hci_event_conn_complete( evt_conn_complete* evt )
{
}

void LocalDevice::Private::hci_event_disconn_complete( evt_disconn_complete* evt )
{
}


/*
*/

LocalDevice::LocalDevice( const char* dev_name )
:	pvt( new Private(this, hci_devid(dev_name)) )
{
	up();
	pvt->init();
}

LocalDevice::LocalDevice( int dev_id )
:	pvt( new Private(this, dev_id) )
{
	up();
	pvt->init();
}

LocalDevice::~LocalDevice()
{
}

void* LocalDevice::data()
{
	return pvt->data;
}

void LocalDevice::data( void* p )
{
	pvt->data = p;
}

Socket& LocalDevice::descriptor()
{
	return pvt->dd;
}

int LocalDevice::id() const	
{
	return pvt->id;
}

const BdAddr& LocalDevice::addr() const
{
	return pvt->ba;
}

/*
*/

int __hci_devinfo( int dd, int id, hci_dev_info* di )
{
	di->dev_id = id;
	return ioctl(dd, HCIGETDEVINFO, (void*)di);
}

/*	device properties
*/
int LocalDevice::get_stats
(
	int* status,
	int* rx_bytes, int* rx_errors,
	int* tx_bytes, int* tx_errors
)
{
	hci_dbg_enter();

	Socket sock;

	hci_dev_info di;
	if(__hci_devinfo(sock.handle(),id(),&di) < 0)
	{
		hci_dbg_leave();
		return -1;
	}

	hci_dev_stats* ds = &di.stat;

	if(status)	*status = hci_test_bit(HCI_UP, &(di.flags));
	if(rx_bytes)	*rx_bytes = ds->byte_rx;
	if(rx_errors)	*rx_errors = ds->err_rx;
	if(tx_bytes)	*tx_bytes = ds->byte_tx;
	if(tx_errors)	*tx_errors = ds->err_tx;

	hci_dbg_leave();

	return 0;
}

int LocalDevice::get_link_mode( u32* lm )
{
	hci_dbg_enter();

	Socket sock;

	hci_dev_info di;
	if(__hci_devinfo(sock.handle(),id(),&di) < 0)
	{
		hci_dbg_leave();
		return -1;
	}
	if(lm)	*lm = di.link_mode;

	hci_dbg_leave();

	return 0;
}

int LocalDevice::set_link_mode( u32 lm )
{
	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = lm;

	if( ioctl(pvt->dd.handle(), HCISETLINKMODE, (ulong)&dr) < 0 )
	{
		return -1;
	}
	return 0;
}

int LocalDevice::get_link_policy( u32* lp )
{
	hci_dbg_enter();

	Socket sock;

	hci_dev_info di;
	if(__hci_devinfo(sock.handle(),id(),&di) < 0)
	{
		hci_dbg_leave();
		return -1;
	}
	if(lp)	*lp = di.link_policy;

	hci_dbg_leave();

	return 0;
}

int LocalDevice::set_link_policy( u32 lp )
{
	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = lp;

	if( ioctl(pvt->dd.handle(), HCISETLINKPOL, (ulong)&dr) < 0 )
	{
		return -1;
	}
	return 0;
}

void LocalDevice::set_auth_enable( u8 enable, void* cookie, int timeout )
{
	u8* auth = new u8;
	*auth = enable;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_AUTH_ENABLE;
	req->hr.cparam = auth;
	req->hr.clen   = 1;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}
void LocalDevice::get_auth_enable( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_AUTH_ENABLE;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);

/*	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_AUTH, &di.flags);
*/
}

void LocalDevice::set_encrypt_mode( u8 encrypt, void* cookie, int timeout )
{
	u8* mode = new u8;
	*mode = encrypt;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_ENCRYPT_MODE;
	req->hr.cparam = mode;
	req->hr.clen   = 1;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}
void LocalDevice::get_encrypt_mode( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_ENCRYPT_MODE;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);

/*	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_ENCRYPT, &di.flags);
*/
}

#if 0
void LocalDevice::set_secman_enable( bool enable )
{
	int sm = enable ? 1 : 0;

	if( ioctl(pvt->dd.handle(), HCISETSECMGR, sm) < 0 )
		throw Exception();
}
bool LocalDevice::get_secman_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_SECMGR, &di.flags);
}
#endif

void LocalDevice::get_scan_enable( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_SCAN_ENABLE;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_SCAN_ENABLE_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);

/*	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_PSCAN, &di.flags);
*/
}

void LocalDevice::set_scan_enable( u8 type, void* cookie, int timeout )
{
	u8* enable = new u8;
	*enable = type;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_SCAN_ENABLE;
	req->hr.cparam = enable;
	req->hr.clen   = 1;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);

/*	struct hci_dev_req dr;

	dr.dev_id  = id();
	dr.dev_opt = iscan_enable() 
			? ( enable ? SCAN_PAGE | SCAN_INQUIRY	: SCAN_INQUIRY )
			: ( enable ? SCAN_PAGE			: SCAN_DISABLED ); 

	if( ioctl(pvt->dd.handle(), HCISETSCAN, (ulong)&dr) < 0 )
		throw Exception();
*/
}

#if 0

void LocalDevice::set_iscan_enable( bool enable, void* cookie, int timeout )
{
/*	struct hci_dev_req dr;

	dr.dev_id  = id();
	dr.dev_opt = pscan_enable() 
			? ( enable ? SCAN_INQUIRY | SCAN_PAGE	: SCAN_PAGE )
			: ( enable ? SCAN_INQUIRY		: SCAN_DISABLED ); 

	if( ioctl(pvt->dd.handle(), HCISETSCAN, (ulong)&dr) < 0 )
		throw Exception();
*/
}
bool LocalDevice::get_iscan_enable( void* cookie, int timeout )
{
/*	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_ISCAN, &di.flags);
*/
}

#endif

void LocalDevice::get_name( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_LOCAL_NAME;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_NAME_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_name( const char* name, void* cookie, int timeout )
{
	change_local_name_cp* cp = new change_local_name_cp;

	memset(cp, 0, sizeof(change_local_name_cp));
	strncpy((char*)cp->name, name, sizeof(cp->name));

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_CHANGE_LOCAL_NAME;
	req->hr.cparam = cp;
	req->hr.clen   = CHANGE_LOCAL_NAME_CP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_class( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_CLASS_OF_DEV;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_CLASS_OF_DEV_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_class( u8 major, u8 minor, u8 x, void* cookie, int timeout )
{
	write_class_of_dev_cp* cp = new write_class_of_dev_cp;
	cp->dev_class[0] = major;
	cp->dev_class[1] = minor;
	cp->dev_class[2] = x;	

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_CLASS_OF_DEV;
	req->hr.cparam = cp;
	req->hr.clen   = WRITE_CLASS_OF_DEV_CP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_voice_setting( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_VOICE_SETTING;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_VOICE_SETTING_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_voice_setting( u16 vs, void* cookie, int timeout )
{
	write_voice_setting_cp* cp = new write_voice_setting_cp;
	cp->voice_setting = vs;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_VOICE_SETTING;
	req->hr.cparam = cp;
	req->hr.clen   = WRITE_VOICE_SETTING_CP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_version( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_LOCAL_VERSION;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_VERSION_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_features( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_LOCAL_FEATURES;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_FEATURES_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}


void LocalDevice::get_address( void* cookie, int timeout )
{
	/*	sometimes we might want to read the address
		even when the device is down, and the function
		below does not work in this case, even if I know
		that adding control flows isn't the best thing
		the device address is an important property
		and IMHO deserves an exception, so...
	*/
	if( pvt->dd.handle() < 0 )
	{
		char local_addr[18] = {0};
		ba2str((bdaddr_t*)pvt->ba.ptr(),local_addr);

		on_get_address(0,cookie,local_addr);
		on_after_event(0,cookie);
		return;
	}

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_BD_ADDR;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_BD_ADDR_RP_SIZE;

	req->to->interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

/*	device operations
*/

void LocalDevice::start_inquiry( u8* lap, u32 flags, void* cookie )
{
	inquiry_cp* cp = new inquiry_cp;

	memset(cp, 0, sizeof(inquiry_cp));
	if(lap)
		memcpy(cp->lap, lap, 3);
	else
	{
		cp->lap[2] = 0x9e;
		cp->lap[1] = 0x8b;
		cp->lap[0] = 0x33;
	}
	cp->length = 0x10;
	cp->num_rsp = 0;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_INQUIRY;
	req->hr.event  = EVT_INQUIRY_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = INQUIRY_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = INQUIRY_INFO_SIZE;

	req->cookie = cookie;
	req->to->interval(25000);

	pvt->post_req(req);
}

void LocalDevice::cancel_inquiry( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_INQUIRY_CANCEL;

	req->cookie = cookie;
	req->to->interval(timeout);

	pvt->post_req(req);
}

void LocalDevice::start_periodic_inquiry( u8* lap, u16 period, void* cookie, int timeout )
{
	periodic_inquiry_cp* cp = new periodic_inquiry_cp;

	memset(cp, 0, sizeof(inquiry_cp));
	if(lap)
		memcpy(cp->lap, lap, 3);
	else
	{
		cp->lap[2] = 0x9e;
		cp->lap[1] = 0x8b;
		cp->lap[0] = 0x33;
	}
	cp->length = 0x10;
	cp->num_rsp = 0;
	cp->max_period = period + 0x20;
	cp->min_period = period - 0x20;

	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_PERIODIC_INQUIRY;
	req->hr.event  = EVT_INQUIRY_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = PERIODIC_INQUIRY_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = INQUIRY_INFO_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);

	pvt->post_req(req);
}

void LocalDevice::cancel_periodic_inquiry( void* cookie, int timeout )
{
	RefPtr<Request> req (new Request);

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_EXIT_PERIODIC_INQUIRY;

	req->cookie = cookie;
	req->to->interval(timeout);

	pvt->post_req(req);
}

/*	inquiry cache
*/

const RemoteDevPTable& LocalDevice::get_inquiry_cache()
{
	return pvt->inquiry_cache;
}

void LocalDevice::clear_cache()
{
	pvt->clear_cache();
}

/*
*/

RemoteDevice::RemoteDevice
(
	LocalDevice* loc_dev,
	RemoteInfo& info
)
:	_local_dev(loc_dev)
{
	update(info);
}

RemoteDevice::~RemoteDevice()
{}


void* RemoteDevice::data()
{
	return _data;
}

void RemoteDevice::data( void* p )
{
	_data = p;
}

bool RemoteDevice::remove_connection( u16 handle )
{
	ConnPTable::iterator i = _connections.find(handle);
	if(i != _connections.end())
	{
		delete i->second;
		_connections.erase(i);
		return true;
	}
	return false;
}

void RemoteDevice::get_name( void* cookie, int timeout )
{
	remote_name_req_cp* cp = new remote_name_req_cp;
	
	memcpy(cp->bdaddr.b,addr().ptr(),sizeof(bdaddr_t));
	cp->pscan_rep_mode = _info.pscan_rpt_mode;
	cp->pscan_mode	   = _info.pscan_mode;
	cp->clock_offset   = _info.clk_offset;

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_REMOTE_NAME_REQ;
	req->hr.event  = EVT_REMOTE_NAME_REQ_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = REMOTE_NAME_REQ_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_REMOTE_NAME_REQ_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}


void RemoteDevice::get_address( void* cookie, int timeout )
{
	const char* address = addr().to_string().c_str();

	this->on_get_address(0, cookie, address);
	_local_dev->on_after_event(0,cookie);
	return;
}


void RemoteDevice::get_class( void* cookie, int timeout )
{
	u8* cls = _info.dev_class;

	this->on_get_class(0, cookie, cls);
	_local_dev->on_after_event(0,cookie);
	return;
}

void RemoteDevice::get_version( void* cookie, int timeout )
{
	if(_connections.empty())
	{
		_local_dev->on_after_event(EIO/*TODO*/,cookie);
		return;
	}
	read_remote_version_cp* cp = new read_remote_version_cp;
	cp->handle = _connections[0]->handle();

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_REMOTE_VERSION;
	req->hr.event  = EVT_READ_REMOTE_VERSION_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_REMOTE_VERSION_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_REMOTE_VERSION_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::get_features( void* cookie, int timeout )
{
	if(_connections.empty())
	{
		_local_dev->on_after_event(EIO/*TODO*/,cookie);
		return;
	}
	read_remote_features_cp* cp = new read_remote_features_cp;
	cp->handle = _connections[0]->handle();

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_REMOTE_FEATURES;
	req->hr.event  = EVT_READ_REMOTE_FEATURES_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_REMOTE_FEATURES_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_REMOTE_FEATURES_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::get_clock_offset( void* cookie, int timeout )
{
	if(_connections.empty())
	{
		_local_dev->on_after_event(EIO/*TODO*/,cookie);
		return;
	}
	read_clock_offset_cp* cp = new read_clock_offset_cp;
	cp->handle = _connections[0]->handle();

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_CLOCK_OFFSET;
	req->hr.event  = EVT_READ_CLOCK_OFFSET_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_CLOCK_OFFSET_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_CLOCK_OFFSET_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::create_acl_connection
(
	u16 ptype,
	bool change_role,
	void* cookie,
	int timeout
)
{
	create_conn_cp* cp = new create_conn_cp;

	memcpy(&(cp->bdaddr), addr().ptr(), 6);
	cp->pkt_type       = ptype;
	cp->pscan_rep_mode = page_scan_repeat_mode();
	cp->clock_offset   = clock_offset();
	cp->role_switch    = change_role ? 1 : 0;

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_CREATE_CONN;
	req->hr.event  = EVT_CONN_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = CREATE_CONN_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_CONN_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::create_sco_connection
(
	u16 handle,
	u16 ptype,
	void* cookie,
	int timeout
)
{
	add_sco_cp* cp = new add_sco_cp;
	cp->handle = handle;
	cp->pkt_type = ptype;

	RefPtr<Request> req (new Request);


	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_ADD_SCO;
	req->hr.event  = EVT_CONN_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = ADD_SCO_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_CONN_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to->interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::update( RemoteInfo& info )
{
	_info = info;

	gettimeofday(&_time_last_updated, NULL);
}

}//namespace Hci
