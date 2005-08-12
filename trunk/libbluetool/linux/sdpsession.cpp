#include <vector>

#include <sys/poll.h>
#include <sys/errno.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "../sdpdebug.h"
#include "../sdpsession.h"
#include "sdprecord_p.h"

namespace Sdp
{

struct Session::Private
{
	BdAddr src;
	BdAddr dst;
	FdNotifier notifier;
	Socket* sock;
	bool local;

	/* read buffer
	*/
	ByteVector ibuf;

	ByteVector tbuf;	//for partial answers
	sdp_pdu_hdr_t* rsp;

	/* write buffer
	*/
	uint obytes;
	ByteVector obuf;
	sdp_pdu_hdr_t* req;

	ByteVector cstate;

	Session* sess;
	bool busy;

	enum
	{
		IDLE,
		INIT,
		CONNECTED,
		WAITING,
		DISCONNECTED
	} state;

	void can_read( FdNotifier& );
	void can_write( FdNotifier& );
	void timed_out( Timeout& );

	void start_send();
	int save_and_cstate();
	void ensure_connected();

	Private( Session* s )
	{
		state = Private::DISCONNECTED;
		sock = NULL;
		sess = s;

		notifier.can_read.connect( sigc::mem_fun(this, &Private::can_read) );
		notifier.can_write.connect( sigc::mem_fun(this, &Private::can_write) );
	}
};

void Session::Private::start_send()
{
	/* init session state
	*/
	//obuf.clear();
	obytes = 0;
	tbuf.clear();
	ibuf.clear();

	ensure_connected();

	notifier.flags( POLLOUT );
}

void Session::Private::ensure_connected()
{
	if( state == CONNECTED )
		return;

	delete sock;

	state = INIT;

	if( local )
	{
		UnixSocket* usock = new UnixSocket;

		if( !usock->connect(SDP_UNIX_PATH) )
		{
			delete usock;
			throw Error();
		}

		sock = usock;
	}
	else
	{
		L2CapSocket* lsock = new L2CapSocket;

		if
		(    !lsock->bind(src)
		  || !lsock->set_blocking(false)
		  || !lsock->connect(dst,0/*TODO:flags*/)
		)
		{
			delete lsock;
			throw Error();
		}

		sock = lsock;
	}
	notifier.fd(sock->handle());
}

int Session::Private::save_and_cstate()
{
	u8* tdata = &tbuf[0] + sizeof(sdp_pdu_hdr_t);
	u16 param_len;

	switch( rsp->pdu_id )
	{
		case SDP_SVC_SEARCH_RSP:
		{
			/* get service record list length
			*/
			//void* ptr = tdata+sizeof(u16);
			//u16 current_records = ntohs(bt_get_unaligned((u16*)ptr)); //TODO: does not compile
			u16 current_records = ntohs(*(u16*)(tdata+sizeof(u16)));
			param_len = current_records*4;
			break;
		}
		case SDP_SVC_SEARCH_ATTR_RSP:
		case SDP_SVC_ATTR_RSP:
		{
			/* get attribute list(s) length
			*/
			//param_len = ntohs(bt_get_unaligned((u16*)tdata)); //TODO: does not compile
			param_len = ntohs(*((u16*)tdata));
			break;
		}
		default:
			throw Error("invalid pdu id");
	}
	/* copy buffered data
	*/
	if(param_len + sizeof(sdp_pdu_hdr_t) > tbuf.size())
	{
		throw Error("pdu too large");
	}
	ibuf.insert(ibuf.end(), tdata, tdata+param_len);

	/* save continuation state
	*/
	tdata += param_len+1;
	u8 cstate_len = *tdata;

	if(cstate_len)
		cstate.assign(tdata, tdata+cstate_len);

	tbuf.clear();

	return cstate_len;
}

void Session::Private::can_read( FdNotifier& )
{
	switch( state )
	{
		case WAITING:
		{
			u8 sf[64];
			int len = sock->recv((char*)sf, sizeof(sf));
			if( len < 0 )
			{	
				if( errno != EAGAIN && errno != EINTR )
				{
					state = DISCONNECTED;
					goto r_disconnected;
				}
			}
			else
			{
				tbuf.insert(tbuf.end(), sf, sf+len);

				if( tbuf.size() >= sizeof(sdp_pdu_hdr_t) )
				{
					rsp = (sdp_pdu_hdr_t*)&tbuf[0];
					
					if( tbuf.size() >= rsp->plen + sizeof(sdp_pdu_hdr_t) )
					{
						/* find continuation state, if any
						*/
						if(save_and_cstate())
						{
							notifier.flags( POLLOUT );
						}
						else
						{
							notifier.flags( 0 );
						}
					}
					
				}
			}
			break;
		}
r_disconnected:	case DISCONNECTED:
		{
			break;
		}
		default:
			break;
	};
}

void Session::Private::can_write( FdNotifier& )
{
	switch( state )
	{
		case INIT:
		{
			state = CONNECTED;
			//fall thru
		}
		case CONNECTED:
		{
			if(obuf.size() - sizeof(sdp_pdu_hdr_t) == 0) //empty request
			{
				/* append arguments
				*/
				if(!cstate.empty())
				{
					/* parse arglist
					*/
					switch( req->pdu_id )
					{
					}
				}
				else
				{
					/* send previous continuation state
					*/
					obuf.insert(obuf.end(),cstate.begin(),cstate.end());
				}

				/* calc req size & set headers
				*/
				req->tid = 0;//__sdp_new_tid(); TODO
				req->plen = obuf.size() - sizeof(sdp_pdu_hdr_t);
			}

			int len = sock->send((char*)&obuf[obytes], obuf.size() - obytes);
			if( len < 0 )
			{
				if( errno != EAGAIN && errno != EINTR )
				{
					state = DISCONNECTED;
					goto w_disconnected;
				}
			}
			else
			{
				obytes += len;
				if( obytes >= obuf.size() )
				{
					//deflate buffer
					ByteVector small(sizeof(sdp_pdu_hdr_t));
					small.swap(obuf);

					state = WAITING;
					notifier.flags( POLLIN );
				}
			}
		break;
		}
w_disconnected:	case DISCONNECTED:
		{
			break;
		}
		default:
			break;
	};
}

Session::Session()
{
	pvt = new Private(this);
	pvt->local = true;
}

Session::Session( BdAddr& src, BdAddr& dest )
{
	pvt = new Private(this);
	pvt->src = src;
	pvt->dst = dest;
	pvt->local = false;
}

Session::~Session()
{
	delete pvt->sock;
	delete pvt;
}

void Session::start_service_search( DataElementSeq& service_pattern )
{
	if( pvt->state != Session::Private::IDLE )
		throw Error(EAGAIN);

	/* start writing header
	   (start_send will do the rest)
	*/
	pvt->req->pdu_id = SDP_SVC_SEARCH_REQ;

	/* append parameters
	*/

	/* start request
	*/
	pvt->start_send();
}

void Session::start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes )
{
	//if( pvt->state != Session::Private::IDLE )
	//	throw Error(EAGAIN);

	sdp_data_t* svc_list = DataElement::Private::new_seq(service_pattern);

	sdp_data_t* attr_list = DataElement::Private::new_seq(attributes);

	pvt->obuf.resize(SDP_REQ_BUFFER_SIZE);

	sdp_buf_t buf;
	buf.data = (char*)&(pvt->obuf[0]);
	buf.data_size = sizeof(sdp_pdu_hdr_t);
	buf.buf_size = pvt->obuf.size();

	sdp_append_to_pdu(&buf,svc_list);
	sdp_append_to_pdu(&buf,attr_list);

	pvt->req = (sdp_pdu_hdr_t*)buf.data;
	pvt->req->pdu_id = SDP_SVC_SEARCH_ATTR_REQ;

	pvt->start_send();

	free(svc_list);
	free(attr_list);
}

/*	search for ALL attributes in ALL services
*/
void Session::start_complete_search()
{
	u16 public_group = PUBLIC_BROWSE_GROUP;

	Sdp::UUID u1 = Sdp::UUID(public_group);
	Sdp::UUID u2 (u1);

	DataElementSeq svcs;
	svcs.push_back( u2 );

	DataElementSeq attrs;
	attrs.push_back( Sdp::U32(0x0000FFFF) );

	start_attr_serv_search(svcs,attrs);
}


}//namespace Sdp
