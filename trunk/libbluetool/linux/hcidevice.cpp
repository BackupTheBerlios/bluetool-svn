#include "hcidevice_p.h"

namespace Hci
{

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

/*	device control
*/

void LocalDevice::up( int dev_id )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVUP, dev_id) < 0 && errno != EALREADY )
		throw Exception();
}

void LocalDevice::down( int dev_id )
{
	Socket sock;
	if( ioctl(sock.handle(), HCIDEVDOWN, dev_id) < 0 )
		throw Exception();
}

void LocalDevice::reset( int dev_id )
{
	up(dev_id);
	down(dev_id);
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

	dd.set_filter(f);
}

void LocalDevice::Private::post_req( Request* req )
{
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
	fire_event(r);
	//notifier.flags( notifier.flags() | POLLOUT );	//force dispatch queue flush
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
					errno = EIO;
					throw Exception(); //hmm, not a good idea
				}
				break;

			case EVT_CMD_COMPLETE:

				cc = (evt_cmd_complete *) ptr;

				if (cc->opcode != pr->ch.opcode)
					break;

				pr->hr.event = EVT_CMD_COMPLETE; //here or post_req() ?

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
				goto _write;
			}
			case Request::QUEUED:
			{
				if(	pr->hr.event == EVT_CMD_STATUS || 
					pr->hr.event == EVT_CMD_COMPLETE &&
					of.opcode() ||
					of.test_event(pr->hr.event)
				)
					break;

				pr->status = Request::WRITING;
				goto _write;
			}
			case Request::TIMEDOUT:
			{	//timeouts are supposed to be in the other queue
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

	if( writev(dd.handle(), pr->iobuf, pr->ion) < 0 )
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

static int __dev_id( const char* name )
{
	if( !strncmp( name, "hci", 3 ) )
		return atoi( name+3 );
	else
		return -1;
}

LocalDevice::LocalDevice( const char* dev_name )
{
	pvt = new Private;
	pvt->parent = this;
	pvt->open(__dev_id(dev_name));
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
void LocalDevice::auth_enable( bool enable )
{
	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = enable ? AUTH_ENABLED : AUTH_DISABLED;

	if( ioctl(pvt->dd.handle(), HCISETAUTH, (ulong)&dr) < 0 )	
		throw Exception();
}
bool LocalDevice::auth_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_AUTH, &di.flags);
}

void LocalDevice::encrypt_enable( bool encrypt )
{
	struct hci_dev_req dr;

	dr.dev_id = id();
	dr.dev_opt = encrypt ? ENCRYPT_P2P : ENCRYPT_DISABLED;

	if( ioctl(pvt->dd.handle(), HCISETENCRYPT, (ulong)&dr) < 0 )
		throw Exception();
}
bool LocalDevice::encrypt_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_ENCRYPT, &di.flags);
}

void LocalDevice::secman_enable( bool enable )
{
	int sm = enable ? 1 : 0;

	if( ioctl(pvt->dd.handle(), HCISETSECMGR, sm) < 0 )
		throw Exception();
}
bool LocalDevice::secman_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_SECMGR, &di.flags);
}

void LocalDevice::pscan_enable( bool enable )
{
	struct hci_dev_req dr;

	dr.dev_id  = id();
	dr.dev_opt = iscan_enable() 
			? ( enable ? SCAN_PAGE | SCAN_INQUIRY	: SCAN_INQUIRY )
			: ( enable ? SCAN_PAGE			: SCAN_DISABLED ); 

	if( ioctl(pvt->dd.handle(), HCISETSCAN, (ulong)&dr) < 0 )
		throw Exception();
}
bool LocalDevice::pscan_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_PSCAN, &di.flags);
}

void LocalDevice::iscan_enable( bool enable )
{
	struct hci_dev_req dr;


	dr.dev_id  = id();
	dr.dev_opt = pscan_enable() 
			? ( enable ? SCAN_INQUIRY | SCAN_PAGE	: SCAN_PAGE )
			: ( enable ? SCAN_INQUIRY		: SCAN_DISABLED ); 

	if( ioctl(pvt->dd.handle(), HCISETSCAN, (ulong)&dr) < 0 )
		throw Exception();
}
bool LocalDevice::iscan_enable()
{
	hci_dev_info di;

	if( __hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_ISCAN, &di.flags);
}

void LocalDevice::local_name( int timeout, void* cookie )
{
	read_local_name_rp* rp = new read_local_name_rp;

	Request* req = new Request;
	req->hr.ogf    = OGF_HOST_CTL;
	req->hr.ocf    = OCF_READ_LOCAL_NAME;
	req->hr.rparam = rp;
	req->hr.rlen   = READ_LOCAL_NAME_RP_SIZE;

	req->to.interval(timeout);
	req->cookie = cookie;

	pvt->post_req(req);
}

void LocalDevice::local_name( const char* name, int timeout, void* cookie )
{
	change_local_name_cp* cp = new change_local_name_cp;

	memset(cp, 0, sizeof(*cp));
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
