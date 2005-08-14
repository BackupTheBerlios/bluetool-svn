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

	RecordList records;

	/* write buffer
	*/
	sdp_buf_t obuf;
	char obuf_array[1024];
	int obytes;
	sdp_pdu_hdr_t* req;

	ByteVector cstate;

	Session* sess;
	bool busy;

	enum
	{
		IDLE,
		SENDING,
		WAITING,
		RECEIVING,
		DISCONNECTED
	} state;

	void can_read( FdNotifier& );
	void can_write( FdNotifier& );
	void timed_out( Timeout& );

	void start_send();
	int save_and_cstate();
	void ensure_connected();
	void response_complete();

	Private( Session* s )
	{
		state = DISCONNECTED;
		sock = NULL;
		sess = s;

		obuf.data = obuf_array;
		obuf.buf_size = sizeof(obuf_array);

		notifier.can_read.connect( sigc::mem_fun(this, &Private::can_read) );
		notifier.can_write.connect( sigc::mem_fun(this, &Private::can_write) );
	}

	~Private()
	{
		delete sock;
	}
};

void Session::Private::start_send()
{
	/* init session state
	*/
	tbuf.clear();
	ibuf.clear();
	records.clear();

	ensure_connected();
}

void Session::Private::ensure_connected()
{
	if( state == SENDING || state == WAITING || state == RECEIVING )
		return;

	delete sock;

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

		if( !lsock->bind(src)
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
	notifier.flags( POLLOUT );

	state = IDLE;
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

			//u16 total_records = ntohs(*(u16*)(tdata));
			tdata += sizeof(u16);
			u16 current_records = ntohs(*(u16*)(tdata+sizeof(u16)));
			tdata += sizeof(u16);
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
			tdata += sizeof(u16);
			break;
		}
		default:
			throw Error("invalid pdu id");
	}
	/* copy buffered data
	*/
	if( tdata + param_len > &(tbuf[0])+tbuf.size() )
	{
		throw Error("pdu too large");
	}
	ibuf.insert(ibuf.end(), tdata, tdata+param_len);

	/* save continuation state
	*/
	tdata += param_len;
	u8 cstate_len = *tdata;

	if(cstate_len)
		cstate.assign(tdata, tdata+cstate_len);

	tbuf.clear();

	return cstate_len;
}

void Session::Private::response_complete()
{
	u8* pdata = &(ibuf[0]);

	switch( rsp->pdu_id )
	{
		case SDP_SVC_SEARCH_RSP:
		{
			while(pdata < &(ibuf[ibuf.size()]))
			{
				u32 handle = ntohl(*(u32*)pdata);

				Record::Private* p = new Record::Private;
				p->alloc = true;
				p->rec = sdp_record_alloc();
				p->rec->handle = handle;

				records.push_back(Record(p));

				pdata += sizeof(u32);
			}
			break;
		}
		case SDP_SVC_ATTR_RSP:
		case SDP_SVC_SEARCH_ATTR_RSP:
		{
			size_t seqlen = 0;

			do
			{
				int len;

				Record::Private* p = new Record::Private;
				p->alloc = true;
				p->rec = sdp_extract_pdu((char*)pdata,&len);

				if(!p->rec) throw Error("invalid pdu in answer");

				records.push_back(Record(p));

				seqlen += len;
				pdata += len;
			}
			while( seqlen < ibuf.size() );

			break;
		}
	}
	sess->on_response(0,records);
}

void Session::Private::can_read( FdNotifier& )
{
	switch( state )
	{
		case WAITING:
		{
			state = RECEIVING;
		}
		case RECEIVING:
		{
			u8 sf[256];
			int len = sock->recv((char*)sf, sizeof(sf));
			if( len < 0 )
			{	
				if( errno != EAGAIN && errno != EINTR )
				{
					state = DISCONNECTED;
					goto r_disconn;
				}
			}
			else
			{
				tbuf.insert(tbuf.end(), sf, sf+len);

				if( tbuf.size() >= sizeof(sdp_pdu_hdr_t) )
				{
					rsp = (sdp_pdu_hdr_t*)&tbuf[0];

					if(rsp->pdu_id == SDP_ERROR_RSP)
						goto sdp_error;

					if(rsp->tid != req->tid)
						break; //not our stuff, ignore it

					rsp->plen = ntohs(rsp->plen);
					
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

							/* send results back
							*/
							response_complete();

							state = IDLE;
						}
					}
					
				}
			}
			break;
		}
r_disconn:	case DISCONNECTED:
		{
			break;
		}
		default:
			break;
	};
	return;

sdp_error:

	u16 error = ntohs(*(u16*)(&tbuf[0]+sizeof(sdp_pdu_hdr_t)));
	sess->on_response(error, records);
}

void Session::Private::can_write( FdNotifier& )
{
	switch( state )
	{
		case IDLE:
		{
			state = SENDING;
			//fall thru
		}
		case SENDING:
		{
			if(obytes == 0) //first time
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
					//obuf.insert(obuf.end(),cstate.begin(),cstate.end());
				}

				/* calc req size & set headers
				*/
				req->tid = htons(13);//__sdp_new_tid(); TODO
				req->plen = htons(obuf.data_size - sizeof(sdp_pdu_hdr_t));
			}

			int len = sock->send(obuf.data+obytes, obuf.data_size - obytes);
			if( len < 0 )
			{
				if( errno != EAGAIN && errno != EINTR )
				{
					state = DISCONNECTED;
					goto w_disconn;
				}
			}
			else
			{
				obytes += len;
				if( obytes >= obuf.data_size )
				{
					state = WAITING;
					notifier.flags( POLLIN );
				}
			}
		break;
		}
w_disconn:	case DISCONNECTED:
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

void Session::start_attribute_search( u32 service_handle, DataElementSeq& attributes )
{
	sdp_data_t* attr_list = DataElement::Private::new_seq(attributes);

	sdp_buf_t* pbuf = &(pvt->obuf);

	pbuf->data_size = sizeof(sdp_pdu_hdr_t);
	pvt->obytes = 0;

	/* service handle
	*/
	*(u32*)(pbuf->data + pbuf->data_size) = htonl(service_handle);
	pbuf->data_size += sizeof(u32);

	/* max responses allowed
	*/
	*(u16*)(pbuf->data + pbuf->data_size) = htons(0xFFFF);
	pbuf->data_size += sizeof(u16);

	/* attribute id list
	*/
	sdp_gen_pdu(pbuf,attr_list);

	/* null continuation state
	*/
	*(u8*)(pbuf->data + pbuf->data_size) = 0;
	pbuf->data_size += sizeof(u8);

	pvt->req = (sdp_pdu_hdr_t*)pbuf->data;
	pvt->req->pdu_id = SDP_SVC_ATTR_REQ;

	pvt->start_send();

	free(attr_list);
}

void Session::start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes )
{
	if( pvt->state != Session::Private::IDLE )
		throw Error(EAGAIN);

	sdp_data_t* svc_list = DataElement::Private::new_seq(service_pattern);
	sdp_data_t* attr_list = DataElement::Private::new_seq(attributes);

	sdp_buf_t* pbuf = &(pvt->obuf);

	pbuf->data_size = sizeof(sdp_pdu_hdr_t);
	pvt->obytes = 0;

	/* add service search pattern
	*/
	sdp_gen_pdu(pbuf,svc_list);

	/* max responses allowed
	*/
	*(u16*)(pbuf->data + pbuf->data_size) = htons(0xFFFF);
	pbuf->data_size += sizeof(u16);

	/* attribute id list
	*/
	sdp_gen_pdu(pbuf,attr_list);

	/* null continuation state
	*/
	*(u8*)(pbuf->data + pbuf->data_size) = 0;
	pbuf->data_size += sizeof(u8);

	pvt->req = (sdp_pdu_hdr_t*)pbuf->data;
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

	DataElementSeq svcs;
	svcs.push_back( Sdp::UUID(public_group) );

	DataElementSeq attrs;
	attrs.push_back( Sdp::U32(0x0000FFFF) );

	start_attr_serv_search(svcs,attrs);
}


}//namespace Sdp
