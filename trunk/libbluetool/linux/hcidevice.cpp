#include "../hcidevice.h"

#include <cstring>

#include <sys/ioctl.h>
#include <sys/errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

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

/* internal representation of a hci request
*/

struct Request;
typedef std::list<Request*> Requests;

/*
*/

struct Request
{
	u16	ogf;
	u16	ocf;
	int	event;
	void*	cmd;
	int	clen;
	EventPacket* evt;
	int	elen;

	/*	buffering
	*/	
	iovec iobuf[3];
	int ion;
	u8 type;
	hci_command_hdr ch;

	/* synchronization
	*/
	Timeout to;

	/* status
	*/
	
	enum
	{	QUEUED,		
		WRITING,
		WAITING,
		TIMEDOUT,
		COMPLETE
	} status;

	/* caller tracking
	*/
	void* cookie;
	
	/* queue position
	*/
	Requests::iterator iter;
};


struct LocalDevice::Private
{
	void open( int dev_id );

	void post_req( Request* );
	void fire_event( Request*, Filter& );

	void read_ready( FdNotifier& );
	void write_ready( FdNotifier& );

	void req_timedout( Timeout& );

	/**/

	Socket	dd;
	int	id;

	FdNotifier	notifier;
	Requests	dispatchq;
	Requests	waitq;

	LocalDevice*	parent;
};

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
	req->ch.opcode = htobs(cmd_opcode_pack( req->ogf, req->ocf ));
	req->ch.plen = req->clen;
	req->iobuf[0].iov_base	= &req->type;
	req->iobuf[0].iov_len	= 1;
	req->iobuf[1].iov_base	= &req->ch;
	req->iobuf[1].iov_len	= HCI_COMMAND_HDR_SIZE;
	if( req->clen )
	{
		req->iobuf[2].iov_base	= req->cmd;
		req->iobuf[2].iov_len	= req->clen;
		req->ion = 3;
	}
	else req->ion = 2;

	req->to.data( req );
	req->to.timed_out.connect( sigc::mem_fun( this, &Private::req_timedout ));

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
	Filter f;
	dd.get_filter(f);
	fire_event( r, f );
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
		throw Exception();
	}
	eh = (hci_event_hdr *) (buf + 1);
	ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
	len -= (1 + HCI_EVENT_HDR_SIZE);

	Requests::reverse_iterator ri = waitq.rbegin();
	while( ri != waitq.rend() )
	{
		if( (*ri)->status == Request::WAITING )
		{
			switch (eh->evt) {

			case EVT_CMD_STATUS:

				cs = (evt_cmd_status *) ptr;

				if (cs->opcode != (*ri)->ch.opcode)
					break;

				if (cs->status)
				{
					errno = EIO;
					throw Exception(); //hmm, not a good idea
				}
				break;

			case EVT_CMD_COMPLETE:

				cc = (evt_cmd_complete *) ptr;

				if (cc->opcode != (*ri)->ch.opcode)
					break;

				ptr += EVT_CMD_COMPLETE_SIZE;
				len -= EVT_CMD_COMPLETE_SIZE;

				(*ri)->elen = MIN(len, (*ri)->elen);
				(*ri)->evt = (EventPacket*) malloc((*ri)->elen);
				memcpy((*ri)->evt, ptr, (*ri)->elen);
				goto _fire;

			default:
				if (eh->evt != (*ri)->event)
					break;

				(*ri)->elen = MIN(len, (*ri)->elen);
				(*ri)->evt = (EventPacket*) malloc((*ri)->elen);
				memcpy((*ri)->evt, ptr, (*ri)->elen);
				goto _fire;
			}
		}
		++ri;
	}
	return;
_fire:	Filter of;
	dd.get_filter(of);
	fire_event( (*ri), of );
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
	while( ri != dispatchq.rend() )
	{
		switch( (*ri)->status )
		{
			case Request::WRITING:
			{
				goto _write;
			}
			case Request::QUEUED:
			{
				if(	(*ri)->event == EVT_CMD_STATUS || 
					(*ri)->event == EVT_CMD_COMPLETE &&
					of.opcode() ||
					of.test_event((*ri)->event)
				)
					break;

				(*ri)->status = Request::WRITING;
				goto _write;
			}
			case Request::TIMEDOUT:
			{	//timeouts are supposed to be on the other queue
				(*ri)->status = Request::WAITING;
				dispatchq.erase( (*ri)->iter );
				waitq.push_front(*ri);
				(*ri)->iter = waitq.begin();

				fire_event( (*ri), of );
				break;

			}
			case Request::WAITING:
			case Request::COMPLETE:
			;	//do nothing
		}
		++ri;
	}
	return;

_write:	if(	(*ri)->event == EVT_CMD_STATUS || 
		(*ri)->event == EVT_CMD_COMPLETE	
	)
		of.set_opcode( htobs(cmd_opcode_pack((*ri)->ogf, (*ri)->ocf)) );
	else
		of.set_event( (*ri)->event );

	dd.set_filter(of);

	if( writev(dd.handle(), (*ri)->iobuf, (*ri)->ion) < 0 )
	{
		if (errno == EAGAIN || errno == EINTR)
			return;
	}
	else
	{
		(*ri)->status = Request::WAITING;
		dispatchq.erase( (*ri)->iter );
		waitq.push_front(*ri);
		(*ri)->iter = waitq.begin();
	}
}

void LocalDevice::Private::fire_event( Request* req, Filter& of )
{
	if( req->status == Request::TIMEDOUT )
	{
		parent->on_event( *(req->evt), req->cookie, true );
	}
	else if( req->status == Request::COMPLETE )
	{
		parent->on_event( *(req->evt), req->cookie, false );
	}
	else return;

	if(	req->event == EVT_CMD_STATUS || 
		req->event == EVT_CMD_COMPLETE	
	)
		of.clear_opcode();
	else
		of.clear_event( req->event );

	dd.set_filter(of);

	free( req->evt );	//we used malloc here
	delete[] (u8*)req->cmd;	//TODO: creepy!
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

int _hci_devinfo( int dd, int id, hci_dev_info* di )
{
	di->dev_id = id;
	return ioctl(dd, HCIGETDEVINFO, (void*)di);
}

/*	device control, todo: the file handle must be unbound?
*/
void LocalDevice::up()
{
	if( ioctl(pvt->dd.handle(), HCIDEVUP, id()) < 0 && errno != EALREADY )
		throw Exception();
}

void LocalDevice::down()
{
	if( ioctl(pvt->dd.handle(), HCIDEVDOWN, id()) < 0 )
		throw Exception();
}

void LocalDevice::reset()
{
	up();
	down();
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

	if( _hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
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

	if( _hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
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

	if( _hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
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

	if( _hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
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

	if( _hci_devinfo(pvt->dd.handle(), id(), &di) < 0 )
		throw Exception();

	return hci_test_bit(HCI_ISCAN, &di.flags);
}

static char __dev_name[128];

const char* LocalDevice::local_name()
{
	if( hci_read_local_name(pvt->dd.handle(), sizeof(__dev_name), __dev_name, 1000) < 0 )
		throw Exception();

	return __dev_name; //shared pointer, multithread nightmare, shouldn't be a problem anyway
}

void LocalDevice::local_name( const char* name )
{
	if( hci_write_local_name(pvt->dd.handle(), name, 2000) < 0 )
		throw Exception();
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
	req->ogf    = OGF_LINK_CTL;
	req->ocf    = OCF_INQUIRY;
	req->event  = EVT_INQUIRY_RESULT;
	req->cmd    = cp;
	req->clen   = INQUIRY_CP_SIZE;
	req->evt    = NULL;
	req->elen   = INQUIRY_INFO_SIZE;
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
