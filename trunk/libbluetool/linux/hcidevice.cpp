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

	Socket sock;
	if( ioctl(sock.handle(), HCIDEVUP, id()) < 0 && errno != EALREADY )
		throw Exception();

	hci_dbg_leave();
}

void LocalDevice::on_up()
{
	hci_dbg_enter();

	hci_dev_info di;
	do
	{
		if(hci_devinfo(id(), &di) < 0)
			throw Exception();
	}
	while(!hci_test_bit(HCI_UP, &di.flags));
	//no, I don't like it, but I'm in a fucking hurry

	usleep(1000000);

	pvt->dd.renew();
	pvt->init();

	hci_dbg_leave();
}

void LocalDevice::down()
{
	hci_dbg_enter();

	Socket sock;
	if( ioctl(sock.handle(), HCIDEVDOWN, id()) < 0 )
		throw Exception();

	hci_dbg_leave();
}

void LocalDevice::on_down()
{
	hci_dbg_enter();

	pvt->dd.close();
	delete pvt->notifier;
	pvt->notifier = NULL;

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
}

/*
*/

void LocalDevice::Private::init()
{
	hci_dbg_enter();

	if( dd.handle() < 0 || !dd.bind(id) || hci_devba(id,(bdaddr_t*)&ba) < 0 )
		throw Exception();

	delete notifier;
	notifier = new FdNotifier();
	notifier->fd( dd.handle() );
	notifier->flags( POLLIN );
	notifier->can_read.connect(sigc::mem_fun( this, &LocalDevice::Private::read_ready ));
	notifier->can_write.connect(sigc::mem_fun( this, &LocalDevice::Private::write_ready ));

	Filter f;
	f.set_type(HCI_EVENT_PKT);
	f.set_event(EVT_CMD_STATUS);
	f.set_event(EVT_CMD_COMPLETE);
	f.clear_opcode();

	if(!dd.set_filter(f))
		throw Exception();

	/* when the this device object is created
	   we update the connections/remote tables
	   with connections created before the daemon
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
		if(ri->second->last_updated() < time_last_inquiry)
		{
			delete ri->second;
			inquiry_cache.erase(ri);
		}
		++ri;
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

void LocalDevice::Private::post_req( Request* req )
{
	if(!notifier) //device not available at the moment
	{
		flush_queues();
		return;
	}

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

	req->to.data( req );
	req->to.timed_out.connect( sigc::mem_fun( this, &Private::req_timedout ));
	req->to.start();

	req->status = Request::QUEUED;
	dispatchq.push_front(req);
	req->iter = dispatchq.begin();

	notifier->flags( notifier->flags() | POLLOUT );
}

void LocalDevice::Private::req_timedout( Timeout& t )
{
	hci_dbg_enter();

	Request* r = static_cast<Request*>(t.data());
	r->status = Request::TIMEDOUT;
	r->to.stop();
	fire_event(r); //todo: fails if timeout occurs in dispatch queue

	hci_dbg_leave();
}


void LocalDevice::Private::flush_queues()
{
	hci_dbg_enter();

	/*	we're toast! cancel all pending I/O
	*/
	Requests::iterator i = waitq.begin();
	waitq.merge(dispatchq);
	while(i != waitq.end())
	{
		Requests::iterator n = i;
		++n;
		if(n != waitq.end())
		{
			parent->on_after_event(EIO,(*i)->cookie);
		}
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

	if(fn.state() & POLLERR || fn.state() & POLLHUP)
	{
		flush_queues();
		throw Exception();
	}
	hci_dbg("fn.state() = %d",fn.state());

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
	Request* pr;
	while( ri != waitq.rend() )
	{
		pr = (*ri);
		if( pr->status == Request::WAITING )
		{
			switch (hp->eh.evt) {

			case EVT_CMD_STATUS:
			{
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
	Request* pr;
	while( ri != dispatchq.rend() )
	{
		pr = (*ri);
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
				dispatchq.erase( pr->iter );
				waitq.push_front(pr);
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
			throw Exception();
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

void LocalDevice::Private::fire_event( Request* req )
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
					link_ctl_cmd_complete(req);
					break;
				}
				case OGF_LINK_POLICY:
				{
					link_policy_cmd_complete(req);
					break;
				}
				case OGF_HOST_CTL:
				{
					host_ctl_cmd_complete(req);
					break;
				}
				case OGF_INFO_PARAM:
				{
					info_param_cmd_complete(req);
					break;
				}
				case OGF_STATUS_PARAM:
				{
					status_param_cmd_complete(req);
					break;
				}
			};						
			break;
		}
		case EVT_CMD_STATUS:
		{
			evt_cmd_status* cs = (evt_cmd_status*) req->hr.rparam;
			if(!cs->status)	//no problems
				goto nodel;

			error = bt_error(cs->status);

			//parent->on_status_failed(status, req->cookie);
			//goto after;
			break;
		}
		/*	command-specific events
		*/
		default:
		{
			hci_event_received(req);
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
	delete req;

nodel:	hci_dbg_leave();
	return;
}

void LocalDevice::Private::link_ctl_cmd_complete( Request* req )
{
	hci_dbg_enter();

	switch( req->hr.ocf )
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
			parent->on_inquiry_cancel(0,req->cookie);
			break;
		}
		case OCF_PERIODIC_INQUIRY:
		{
			parent->on_periodic_inquiry_started(0,req->cookie);
			break;
		}
		case OCF_EXIT_PERIODIC_INQUIRY:
		{
			parent->on_periodic_inquiry_cancel(0,req->cookie);
			break;
		}
	}

	hci_dbg_leave();
}

void LocalDevice::Private::link_policy_cmd_complete( Request* req )
{
}

void LocalDevice::Private::host_ctl_cmd_complete( Request* req )
{
	hci_dbg_enter();

	switch( req->hr.ocf )
	{
		case OCF_READ_AUTH_ENABLE:
		{
			u8* enable = (u8*) req->hr.rparam;

			parent->on_get_auth_enable(0, req->cookie, *enable);
			break;
		}
		case OCF_READ_ENCRYPT_MODE:
		{
			u8* mode = (u8*) req->hr.rparam;

			parent->on_get_encrypt_mode(0, req->cookie, *mode);
			break;
		}
		case OCF_READ_SCAN_ENABLE:
		{
			read_scan_enable_rp* r = (read_scan_enable_rp*) req->hr.rparam;

			parent->on_get_scan_enable(r->status,req->cookie,r->enable);
			break;
		}
		case OCF_READ_LOCAL_NAME:
		{
			read_local_name_rp* r = (read_local_name_rp*) req->hr.rparam;

			parent->on_get_name(r->status,req->cookie,(char*)r->name);
			break;
		}
		case OCF_READ_CLASS_OF_DEV:
		{
			read_class_of_dev_rp* r = (read_class_of_dev_rp*) req->hr.rparam;

			parent->on_get_class(r->status,req->cookie,r->dev_class);
			break;
		}
		case OCF_READ_VOICE_SETTING:
		{
			read_voice_setting_rp* r = (read_voice_setting_rp*) req->hr.rparam;

			parent->on_get_voice_setting(r->status,req->cookie,r->voice_setting);
			break;
		}
		case OCF_WRITE_SCAN_ENABLE:
		{
			parent->on_set_scan_enable(0,req->cookie);
			break;
		}
		case OCF_CHANGE_LOCAL_NAME:
		{
			parent->on_set_name(0,req->cookie);
			break;
		}
	}
	hci_dbg_leave();
}

void LocalDevice::Private::info_param_cmd_complete( Request* req )
{
	hci_dbg_enter();

	switch( req->hr.ocf )
	{
		case OCF_READ_BD_ADDR:
		{
			read_bd_addr_rp* r = (read_bd_addr_rp*) req->hr.rparam;

			char local_addr[32] = {0};
			ba2str(&(r->bdaddr),local_addr);
			char* local_addr_ptr = local_addr;

			parent->on_get_address(r->status,req->cookie,local_addr_ptr);
			break;
		}
		case OCF_READ_LOCAL_VERSION:
		{
			read_local_version_rp* r = (read_local_version_rp*) req->hr.rparam;

			const char* hci_ver = hci_vertostr(r->hci_ver);
			const char* lmp_ver = lmp_vertostr(r->lmp_ver);
			const char* comp_str = bt_compidtostr(r->manufacturer);

			parent->on_get_version
			(
				r->status,
				req->cookie,
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
			read_local_features_rp* r = (read_local_features_rp*) req->hr.rparam;

			char* lmp_feat = lmp_featurestostr(r->features," ",0);

			parent->on_get_features(r->status,req->cookie,lmp_feat);
			 //todo: format this field as a string array

			free(lmp_feat);
			break;
		}
	}
	hci_dbg_leave();
}

void LocalDevice::Private::status_param_cmd_complete( Request* req )
{
}

void LocalDevice::Private::hci_event_received( Request* req )
{
	hci_dbg_enter();

	switch(req->hr.event)
	{
		case EVT_INQUIRY_RESULT:
		{
			inquiry_info* r = (inquiry_info*) req->hr.rparam;

			BdAddr addr (r->bdaddr.b);

			RemoteInfo info =
			{
				addr,
				{
					r->dev_class[0],
					r->dev_class[1],
					r->dev_class[2],
				},
				r->pscan_rep_mode,
				r->pscan_period_mode,
				r->pscan_mode,
				r->clock_offset
			};

			update_cache(info);
			break;
		}
		case EVT_INQUIRY_COMPLETE:
		{
			finalize_cache();

			//u16* status = (u16*) req->hr.rparam;

			parent->on_inquiry_complete(0, req->cookie);
			break;
		}
		/*	remote commands
		*/
		case EVT_REMOTE_NAME_REQ_COMPLETE:
		{
			evt_remote_name_req_complete* r = (evt_remote_name_req_complete*) req->hr.rparam;

			char straddr[18] = {0};
			ba2str(&(r->bdaddr),straddr);

			RemoteDevPTable::iterator i = inquiry_cache.find(straddr);

			if( i != inquiry_cache.end() )
			{
				i->second->on_get_name(r->status, req->cookie, (char*)r->name);
			}
			else
			{
				hci_dbg("remote name %s from non cached device %s!",r->name, straddr);
			}
			break;
		}		
		case EVT_READ_REMOTE_VERSION_COMPLETE:
		{
			evt_read_remote_version_complete* r = (evt_read_remote_version_complete*) req->hr.rparam;

			const char* lmp_ver = lmp_vertostr(r->lmp_ver);
			const char* comp_str = bt_compidtostr(r->manufacturer);

			req->src.remote->on_get_version
			(
				r->status,
				req->cookie,
				lmp_ver,
				r->lmp_subver,
				comp_str
			);
			break;
		}
		case EVT_READ_REMOTE_FEATURES_COMPLETE:
		{
			evt_read_remote_features_complete* r = (evt_read_remote_features_complete*) req->hr.rparam;

			char* lmp_feat = lmp_featurestostr(r->features," ",0);
			req->src.remote->on_get_features(r->status, req->cookie, lmp_feat);
			break;
		}
		case EVT_READ_CLOCK_OFFSET_COMPLETE:
		{
			evt_read_clock_offset_complete* r = (evt_read_clock_offset_complete*) req->hr.rparam;

			req->src.remote->on_get_clock_offset(r->status, req->cookie, r->clock_offset);
			break;
		}
		/*	connection
		*/
		case EVT_CONN_COMPLETE:
		{
			evt_conn_complete* r = (evt_conn_complete*) req->hr.rparam;

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
//			req->dest_type = Request::REMOTE;
//			req->dest.rem = i->second;
			break;
		}
		case EVT_DISCONN_COMPLETE:
		{
			evt_disconn_complete* r = (evt_disconn_complete*) req->hr.rparam;

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

/*
*/

LocalDevice::LocalDevice( const char* dev_name )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->id = hci_devid(dev_name);
	pvt->notifier = NULL;

	up();
	pvt->init();
}

LocalDevice::LocalDevice( int dev_id )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->id = dev_id;
	pvt->notifier = NULL;

	up();
	pvt->init();
}

LocalDevice::~LocalDevice()
{
	if(noref())
	{
		delete pvt->notifier;
		delete pvt;
	}
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

	return 1;
}

void LocalDevice::set_auth_enable( u8 enable, void* cookie, int timeout )
{
/*	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = enable ? AUTH_ENABLED : AUTH_DISABLED;

	if( ioctl(pvt->dd.handle(), HCISETAUTH, (ulong)&dr) < 0 )	
		throw Exception();
*/
}
void LocalDevice::get_auth_enable( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_AUTH_ENABLE;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to.interval(timeout);
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
/*	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = encrypt ? ENCRYPT_P2P : ENCRYPT_DISABLED;

	if( ioctl(pvt->dd.handle(), HCISETENCRYPT, (ulong)&dr) < 0 )
		throw Exception();
*/
}
void LocalDevice::get_encrypt_mode( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_ENCRYPT_MODE;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to.interval(timeout);
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
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_SCAN_ENABLE;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_SCAN_ENABLE_RP_SIZE;

	req->to.interval(timeout);
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

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_SCAN_ENABLE;
	req->hr.cparam = enable;
	req->hr.clen   = 1;
	req->hr.rparam = NULL;
	req->hr.rlen   = 1;

	req->to.interval(timeout);
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
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_LOCAL_NAME;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_NAME_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_name( const char* name, void* cookie, int timeout )
{
	change_local_name_cp* cp = new change_local_name_cp;

	memset(cp, 0, sizeof(change_local_name_cp));
	strncpy((char*)cp->name, name, sizeof(cp->name));

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_CHANGE_LOCAL_NAME;
	req->hr.cparam = cp;
	req->hr.clen   = CHANGE_LOCAL_NAME_CP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_class( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_CLASS_OF_DEV;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_CLASS_OF_DEV_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_class( u8 major, u8 minor, u8 x, void* cookie, int timeout )
{
	write_class_of_dev_cp* cp = new write_class_of_dev_cp;
	cp->dev_class[0] = major;
	cp->dev_class[1] = minor;
	cp->dev_class[2] = x;	

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_CLASS_OF_DEV;
	req->hr.cparam = cp;
	req->hr.clen   = WRITE_CLASS_OF_DEV_CP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_voice_setting( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_VOICE_SETTING;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_VOICE_SETTING_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::set_voice_setting( u16 vs, void* cookie, int timeout )
{
	write_voice_setting_cp* cp = new write_voice_setting_cp;
	cp->voice_setting = vs;

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_VOICE_SETTING;
	req->hr.cparam = cp;
	req->hr.clen   = WRITE_VOICE_SETTING_CP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_version( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_LOCAL_VERSION;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_VERSION_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::get_features( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_LOCAL_FEATURES;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_LOCAL_FEATURES_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}


void LocalDevice::get_address( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_INFO_PARAM;
	req->hr.ocf    = OCF_READ_BD_ADDR;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_BD_ADDR_RP_SIZE;

	req->to.interval(timeout);
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

	Request* req = new Request;
	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_INQUIRY;
	req->hr.event  = EVT_INQUIRY_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = INQUIRY_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = INQUIRY_INFO_SIZE;

	req->cookie = cookie;
	req->to.interval(25000);

	pvt->post_req(req);
}

void LocalDevice::cancel_inquiry( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_INQUIRY_CANCEL;

	req->cookie = cookie;
	req->to.interval(timeout);

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

	Request* req = new Request;
	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_PERIODIC_INQUIRY;
	req->hr.event  = EVT_INQUIRY_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = PERIODIC_INQUIRY_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = INQUIRY_INFO_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);

	pvt->post_req(req);
}

void LocalDevice::cancel_periodic_inquiry( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_EXIT_PERIODIC_INQUIRY;

	req->cookie = cookie;
	req->to.interval(timeout);

	pvt->post_req(req);
}


/*
*/

RemoteDevice::RemoteDevice
(
	LocalDevice* loc_dev,
	RemoteInfo& info
)
:	_info(info),
	_local_dev(loc_dev)
{

}

RemoteDevice::~RemoteDevice()
{}

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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_REMOTE_NAME_REQ;
	req->hr.event  = EVT_REMOTE_NAME_REQ_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = REMOTE_NAME_REQ_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_REMOTE_NAME_REQ_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_REMOTE_VERSION;
	req->hr.event  = EVT_READ_REMOTE_VERSION_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_REMOTE_VERSION_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_REMOTE_VERSION_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_REMOTE_FEATURES;
	req->hr.event  = EVT_READ_REMOTE_FEATURES_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_REMOTE_FEATURES_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_REMOTE_FEATURES_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_READ_CLOCK_OFFSET;
	req->hr.event  = EVT_READ_CLOCK_OFFSET_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = READ_CLOCK_OFFSET_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_READ_CLOCK_OFFSET_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_CREATE_CONN;
	req->hr.event  = EVT_CONN_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = CREATE_CONN_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_CONN_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
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

	Request* req = new Request;

	req->hr.ogf    = OGF_LINK_CTL;
	req->hr.ocf    = OCF_ADD_SCO;
	req->hr.event  = EVT_CONN_COMPLETE;
	req->hr.cparam = cp;
	req->hr.clen   = ADD_SCO_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = EVT_CONN_COMPLETE_SIZE;

	req->cookie = cookie;
	req->to.interval(timeout);
	req->src.remote = this;

	_local_dev->pvt->post_req(req);
}

void RemoteDevice::update( RemoteInfo& info )
{
	_info = info;

	timeval now;
	gettimeofday(&now, NULL);

	_time_last_updated = now.tv_sec*1000 + now.tv_usec/1000;
}

}//namespace Hci
