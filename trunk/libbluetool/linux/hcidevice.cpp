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

void LocalDevice::up( int id )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVUP, id) < 0 && errno != EALREADY )
		throw Exception();
}

void LocalDevice::down( int id )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVDOWN, id) < 0 )
		throw Exception();
}

void LocalDevice::reset( int id )
{
	up(id);
	down(id);
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

void LocalDevice::Private::init( int dev_id )
{
	id = dev_id;

	up(id);

	if( dd.handle() < 0 || !dd.bind(id) || hci_devba(id,(bdaddr_t*)&ba) < 0 )
		throw Exception();

	notifier.fd( dd.handle() );
	notifier.flags( POLLIN );
	notifier.can_read.connect(sigc::mem_fun( this, &LocalDevice::Private::read_ready ));
	notifier.can_write.connect(sigc::mem_fun( this, &LocalDevice::Private::write_ready ));

	Filter f;
	f.set_type(HCI_EVENT_PKT);
	f.set_event(EVT_CMD_STATUS);
	f.set_event(EVT_CMD_COMPLETE);
	f.clear_opcode();

	if(!dd.set_filter(f))
		throw Exception();
}

void LocalDevice::Private::update_cache
(
	RemoteInfo& info
)
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

	notifier.flags( notifier.flags() | POLLOUT );
}

void LocalDevice::Private::req_timedout( Timeout& t )
{
	Request* r = static_cast<Request*>(t.data());
	r->status = Request::TIMEDOUT;
	r->to.stop();
	fire_event(r); //todo: fails if timeout occurs in dispatch queue
}

void LocalDevice::Private::read_ready( FdNotifier& fn )
{
	/* ripped from file hci.c in Bluez 2.18
	*/
	u8 buf[HCI_MAX_EVENT_SIZE], *ptr;
	hci_event_hdr    *eh;
	evt_cmd_complete *cc;
	evt_cmd_status   *cs;
	int len;

	if(fn.state() & POLLERR || fn.state() & POLLHUP)
		throw Exception();

	while( (len = read(fn.fd(), buf, sizeof(buf))) < 0 )
	{
        	if (errno == EAGAIN || errno == EINTR)
			continue;
		throw Exception(); // todo, this obviously terminates the program
	}
	eh = (hci_event_hdr *) (buf + 1);
	ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
	len -= (1 + HCI_EVENT_HDR_SIZE);

	Requests::reverse_iterator ri = waitq.rbegin();
	Request* pr;
	while( ri != waitq.rend() )
	{
		pr = (*ri);
		if( pr->status == Request::WAITING )
		{
			switch (eh->evt) {

			case EVT_CMD_STATUS:

				cs = (evt_cmd_status *) ptr;

				if (cs->opcode != pr->ch.opcode)
					break;

				if (cs->status)
				{
					pr->hr.event = EVT_CMD_STATUS;
					fire_event(pr);
				}
				break;

			case EVT_CMD_COMPLETE:

				cc = (evt_cmd_complete *) ptr;

				if (cc->opcode != pr->ch.opcode)
					break;

				ptr += EVT_CMD_COMPLETE_SIZE;
				len -= EVT_CMD_COMPLETE_SIZE;

				pr->hr.rlen = MIN(len, pr->hr.rlen);
				pr->hr.rparam = malloc(pr->hr.rlen);
				memcpy(pr->hr.rparam, ptr, pr->hr.rlen);
				goto _fire;

			default:
				if (eh->evt != pr->hr.event)
					break;

				pr->hr.rlen = MIN(len, pr->hr.rlen);
				pr->hr.rparam = malloc(pr->hr.rlen);
				memcpy(pr->hr.rparam, ptr, pr->hr.rlen);
				goto _fire;
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
	notifier.flags( notifier.flags() | POLLOUT );
}

void LocalDevice::Private::write_ready( FdNotifier& fn )
{
	if(dispatchq.empty())
	{
		notifier.flags( notifier.flags() & ~POLLOUT );
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
						notifier.flags( notifier.flags() & ~POLLOUT );
						break;
					}
				}
				else if(of.test_event(pr->hr.event))
				{
					notifier.flags( notifier.flags() & ~POLLOUT );
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
			return;
		else
			throw Exception();
	}
	else
	{
		pr->status = Request::WAITING;
		dispatchq.erase( pr->iter );
		waitq.push_front(pr);
		pr->iter = waitq.begin();
	}
}

static u16 get_status( Request* req )
{
	if(req->hr.event == EVT_CMD_STATUS)
	{
		evt_cmd_status* cs = (evt_cmd_status*) req->hr.rparam;
		return cs->status;
	}
	else if(req->status == Request::TIMEDOUT)
	{
		return ETIMEDOUT;
	}
	return 0;
}

void LocalDevice::Private::fire_event( Request* req )
{
	Filter of;

	req->dest_type = Request::LOCAL; //this is the default
	req->dest.loc = parent;

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
			u16 status = get_status(req);

			if(!status)	//no problems
				goto nodel;

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
	parent->on_after_event(req->cookie);
	waitq.erase( req->iter );
	delete req;
nodel:	return;
}

void LocalDevice::Private::link_ctl_cmd_complete( Request* req )
{
}

void LocalDevice::Private::link_policy_cmd_complete( Request* req )
{
}

void LocalDevice::Private::host_ctl_cmd_complete( Request* req )
{
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
		case OCF_READ_INQUIRY_SCAN_TYPE:
		{
			read_inquiry_scan_type_rp* r = (read_inquiry_scan_type_rp*) req->hr.rparam;

			parent->on_get_scan_type(r->status,req->cookie,r->type);
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
		case OCF_WRITE_INQUIRY_SCAN_TYPE:
		{
			parent->on_set_scan_type(0,req->cookie);
			break;
		}
		case OCF_CHANGE_LOCAL_NAME:
		{
			parent->on_set_name(0,req->cookie);
			break;
		}
	}
}

void LocalDevice::Private::info_param_cmd_complete( Request* req )
{
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
			const char* lmp_ver = lmp_vertostr(r->hci_ver);
			const char* comp_id = bt_compidtostr(r->manufacturer);

			parent->on_get_version
			(
				r->status,
				req->cookie,
				hci_ver,
				r->hci_rev,
				lmp_ver,
				r->lmp_subver,
				comp_id
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
}

void LocalDevice::Private::status_param_cmd_complete( Request* req )
{
}

void LocalDevice::Private::hci_event_received( Request* req )
{
	switch(req->hr.event)
	{
		case EVT_INQUIRY_RESULT:
		{
			inquiry_info* r = (inquiry_info*) req->hr.rparam;

			BdAddr addr (r->bdaddr.b);

			RemoteInfo info =
			{
				addr,
				r->pscan_rep_mode,
				r->pscan_period_mode,
				r->pscan_mode,
				{
					r->dev_class[0],
					r->dev_class[1],
					r->dev_class[2],
				},
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

//			req->dest_type = Request::REMOTE;
//			req->dest.rem = i->second;
			break;
		}
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
}

/*
*/

LocalDevice::LocalDevice( const char* dev_name )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->init(hci_devid(dev_name));
}

LocalDevice::LocalDevice( int dev_id )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->init(dev_id);
}

LocalDevice::~LocalDevice()
{
	if(noref())	delete pvt;
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

void LocalDevice::get_scan_type( void* cookie, int timeout )
{
	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_INQUIRY_SCAN_TYPE;
	req->hr.rparam = NULL;
	req->hr.rlen   = READ_INQUIRY_SCAN_TYPE_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);

/*	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_PSCAN, &di.flags);
*/
}

void LocalDevice::set_scan_type( u8 type, void* cookie, int timeout )
{
	write_inquiry_scan_type_cp* cp = new write_inquiry_scan_type_cp;

	memset(cp, 0, sizeof(write_inquiry_scan_type_cp));
	cp->type = type;

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_WRITE_INQUIRY_SCAN_TYPE;
	req->hr.cparam = cp;
	req->hr.clen   = WRITE_INQUIRY_SCAN_TYPE_CP_SIZE;;
	req->hr.rparam = NULL;
	req->hr.rlen   = WRITE_INQUIRY_SCAN_TYPE_RP_SIZE;

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

void LocalDevice::set_class( u32 cls, void* cookie, int timeout )
{
	write_class_of_dev_cp* cp = new write_class_of_dev_cp;
	cp->dev_class[0] = cls & 0xff;
	cp->dev_class[1] = (cls >> 8) & 0xff;
	cp->dev_class[2] = (cls >> 16) & 0xff;	

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
	req->to.interval(20000);

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
