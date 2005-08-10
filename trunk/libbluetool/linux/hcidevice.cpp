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

void LocalDevice::up( const char* name )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVUP, hci_devid(name)) < 0 && errno != EALREADY )
		throw Exception();
}

void LocalDevice::down( const char* name )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVDOWN, hci_devid(name)) < 0 )
		throw Exception();
}

void LocalDevice::reset( const char* name )
{
	up(name);
	down(name);
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
	if(hr.cparam) delete[] (u8*)hr.cparam;
	if(hr.rparam) delete[] (u8*)hr.rparam;
}

/*
*/

void LocalDevice::Private::open( int dev_id )
{
	id = dev_id;

	if( dd.handle() < 0 || !dd.bind(id) )
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
					if(of.opcode()) break;
				}
				else if(of.test_event(pr->hr.event))
				{
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
	}
	else
	{
		pr->status = Request::WAITING;
		dispatchq.erase( pr->iter );
		waitq.push_front(pr);
		pr->iter = waitq.begin();
	}
}

void LocalDevice::Private::fire_event( Request* req )
{
	Filter of;

	EventPacket evt;
	evt.code  = req->hr.event;
	evt.ogf   = req->hr.ogf;
	evt.ocf   = req->hr.ocf;
	evt.edata = req->hr.rparam;

	if( req->status == Request::TIMEDOUT )
	{
		parent->on_event( evt, req->cookie, true );
	}
	else if( req->status == Request::COMPLETE )
	{
		parent->on_event( evt, req->cookie, false );
	}
	else goto cleanreq;

	dd.get_filter(of);

	if(	req->hr.event == EVT_CMD_STATUS || 
		req->hr.event == EVT_CMD_COMPLETE	
	)
		of.clear_opcode();
	else
		of.clear_event( req->hr.event );

	dd.set_filter(of);

cleanreq:

	waitq.erase( req->iter );
	delete req;
}

/*
*/

LocalDevice::LocalDevice( const char* dev_name )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->open(hci_devid(dev_name));
}

LocalDevice::LocalDevice( int dev_id )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->open(dev_id);
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

void LocalDevice::get_local_name( void* cookie, int timeout )
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

void LocalDevice::set_local_name( const char* name, void* cookie, int timeout )
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

void LocalDevice::get_addr( void* cookie, int timeout )
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
	req->hr.event  = EVT_INQUIRY_RESULT;
	req->hr.cparam = cp;
	req->hr.clen   = INQUIRY_CP_SIZE;
	req->hr.rparam = NULL;
	req->hr.rlen   = INQUIRY_INFO_SIZE;

	req->cookie = cookie;
	req->to.interval(11000);

	pvt->post_req(req);
}

/*
*/

RemoteDevice::RemoteDevice( const BdAddr& addr, u8 pscan_rpt_mode , u8 pscan_mode, u16 clk_offset )
: _addr(addr), _pscan_rpt_mode(pscan_rpt_mode), _pscan_mode(pscan_mode), _clk_offset(clk_offset)
{

}

RemoteDevice::~RemoteDevice()
{}

}//namespace Hci
