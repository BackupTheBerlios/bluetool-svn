#include <common/refptr_impl.h>
#include <vector>

#include <sys/poll.h>
#include <sys/errno.h>
#include <netinet/in.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "sdprecord_p.h"
#include <bluetool/sdpdebug.h>
#include <bluetool/sdpclient.h>

namespace Sdp
{

static u16 __next_tid = 0;

u16 __sdp_gen_tid()
{
	return ++__next_tid;
}

struct Client::Private
{
	BdAddr src;
	BdAddr dst;
	FdNotifier* notifier;
	RefPtr<Socket> sock;
	bool local;

	/* read buffer
	*/
	ByteVector ibuf;

	//ByteVector tbuf;	//for partial answers
	sdp_pdu_hdr_t* rsp;

	RecordList returned_records;
	RecordList cached_records;

	DataElementSeq service_pattern;
	DataElementSeq attribute_pattern;

	/* write buffer
	*/
	sdp_buf_t obuf;
	sdp_buf_t sbuf;		//send buffer, contains obuf + continuation state

	char obuf_array[1024];
	int obytes;
	sdp_pdu_hdr_t* req;

	ByteVector cstate;

	Client* sess;
	bool busy;

	enum
	{
		IDLE,		//doing nothing
		SENDING,	//sending a request
		WAITING,	//waiting for a response
		RECEIVING,	//reading a (partial)response
		DISCONNECTED
	} state;

	void can_read( FdNotifier& );
	void can_write( FdNotifier& );
	void timed_out( Timeout& );

	void start_send();
	int save_and_cstate( char* read_buf, int read_len );	//save partial response
	void try_connect( Timeout& );
	void response_complete();
	void update_cache();

	Private( Client* s )
	:	notifier(NULL)
	{
		state = DISCONNECTED;
		sess = s;

		obuf.data = obuf_array;
		obuf.buf_size = sizeof(obuf_array);
	}
};

void Client::Private::start_send()
{
	sdp_dbg_enter();

	/* init session state
	*/
	ibuf.clear();
	returned_records.clear();
	//service_pattern.clear();
	//attribute_pattern.clear();

	Timeout* t = Timeout::create(1000);
	t->timed_out.connect( sigc::mem_fun(this, &Client::Private::try_connect) );
	t->start();

	sdp_dbg_leave();
}

void Client::Private::try_connect( Timeout& tick )
{
	sdp_dbg_enter();

	if( state == SENDING || state == WAITING || state == RECEIVING )
		return;

	if( local )
	{
		UnixSocket* usock = new UnixSocket();

		if( !usock->connect(SDP_UNIX_PATH) )
		{
			throw Error();
		}

		sock = RefPtr<Socket>(usock);
	}
	else
	{
		L2CapSocket* lsock = new L2CapSocket();

		if( !lsock->bind(src)
		    || !lsock->connect(dst,0)
		)
		{
			delete lsock;

			if( errno == EBUSY )
			{
				sdp_dbg_leave();
				return;	//try later
			}
			else
			{
				sess->on_read_response(errno,returned_records);
				goto acl_failed;
			}
		}

		sock = RefPtr<Socket>(lsock);
	}
	notifier = FdNotifier::create( sock->handle(), POLLOUT );

	notifier->can_read.connect( sigc::mem_fun(this, &Private::can_read) );
	notifier->can_write.connect( sigc::mem_fun(this, &Private::can_write) );

	state = IDLE;

acl_failed:

	Timeout::destroy(&tick);
	sdp_dbg_leave();
}

int Client::Private::save_and_cstate( char* read_buf, int read_len )
{
	char* tdata = read_buf + sizeof(sdp_pdu_hdr_t);
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
	if( tdata + param_len > read_buf + read_len )
	{
		throw Error("pdu too large");
	}
	ibuf.insert(ibuf.end(), tdata, tdata+param_len);

	/* save continuation state
	*/
	tdata += param_len;
	u8 cstate_len = *tdata;

	if(cstate_len)
		cstate.assign(tdata, tdata+cstate_len+1);
	else
		cstate.clear();

	return cstate_len;
}

void Client::Private::response_complete()
{
	sdp_dbg_enter();

	u8* pdata = &(ibuf[0]);

	switch( rsp->pdu_id )
	{
		case SDP_SVC_SEARCH_RSP:
		{
			while(pdata < &(ibuf[ibuf.size()]))
			{
				u32 handle = ntohl(*(u32*)pdata);

				Record::Private* p = new Record::Private();
				p->alloc = true;
				p->rec = sdp_record_alloc();
				p->rec->handle = handle;

				returned_records.push_back(Record(p));

				pdata += sizeof(u32);
			}
			break;
		}
		case SDP_SVC_ATTR_RSP:
		case SDP_SVC_SEARCH_ATTR_RSP:
		{
			int seqlen = 0;

			u8 dtd;
			uint xtracted = sdp_extract_seqtype((char*)pdata, &dtd, &seqlen);

			if( xtracted <= 0 ) break;

			pdata += xtracted;
			do
			{
				int bread = 0;

				sdp_record_t* rec = sdp_extract_pdu((char*)pdata,&bread);

				if(!rec->attrlist)
				{
					// the specification says records cannot be empty,
					// so we discard records without attributes
					// (btw the servers shouldn't send this stuff at all)
					sdp_record_free(rec);
					break;
				}

				Record::Private* p = new Record::Private();
				p->alloc = true;
				p->rec = rec;

				if(!p->rec) throw Error("invalid pdu in answer"); //mhh, this leaks

				returned_records.push_back(Record(p));

				xtracted += bread;
				pdata += bread;
			}
			while( xtracted < ibuf.size() );

			break;
		}
	}
	/*	now, put the new stuff into the cache
		and remove invalid records
		we do that before sending the response because the
		wrapper might actually expect the fresh data
		to be in the cache already
	*/
	update_cache();

	sess->on_read_response(0,returned_records);

	returned_records.clear();

	sdp_dbg_leave();
}

void Client::Private::update_cache()
{
	/*	1: remove obsolete records
	*/
	RecordList::iterator rit;
	RecordList::iterator cit;

	//	for each 'old' record
	cit = cached_records.begin();
	while( cit != cached_records.end() )
	{

		rit = returned_records.begin();
		while( rit != returned_records.end() )
		{
			if( cit->handle() == rit->handle() )
				break;
			++rit;
		}

	//	if it's NOT among the returned ones
		if( rit == returned_records.end() )
		{
		//	while its attribute IDs say it should be
			if( cit->match(service_pattern) )
			{
			//	assume the record was canceled, and remove it
				RecordList::iterator nit = cit;
				nit++;

				// tell upper levels to free any associated resources
				sess->on_purge_cache_entry(*cit);

				cached_records.erase(cit);
				cit = nit;

				continue;
			}
		}
		++cit;
	}


	/*	2: insert the new data
	*/

	//	for each new record
	rit = returned_records.begin();
	while( rit != returned_records.end() )
	{
		cit = cached_records.begin();
		while( cit != cached_records.end() )
		{
			if( rit->handle() == cit->handle() )
				break;
			++cit;
		}

	//	if we already have a copy...
		if( cit != cached_records.end() )
		{
		//	...update that one
			sess->on_purge_cache_entry(*cit);
			
			cached_records.erase(cit);

			cached_records.push_back(*rit);

			sess->on_new_cache_entry(*rit);
		}
		else
		{
		//	...else push the new record
			cached_records.push_back(*rit);

			// notify upper layer
			sess->on_new_cache_entry(*rit);
		}
		++rit;
	}
}

void Client::Private::can_read( FdNotifier& )
{
	char rbuf[SDP_RSP_BUFFER_SIZE];
			
	sdp_dbg_enter();

	switch( state )
	{
		case WAITING:
		{
			state = RECEIVING;
		}
		case RECEIVING:
		{
			int len = sock->recv(rbuf, sizeof(rbuf));

			if( len <= 0 )
			{
				state = DISCONNECTED;
				goto r_disconn;
			}
			else
			{
				if( (uint)len >= sizeof(sdp_pdu_hdr_t) )
				{
					rsp = (sdp_pdu_hdr_t*)rbuf;

					if(rsp->pdu_id == SDP_ERROR_RSP)
						goto sdp_error;

					if(rsp->tid != req->tid)
						break; //not our stuff, ignore it

					rsp->plen = ntohs(rsp->plen);
					
					if( (uint)len >= rsp->plen + sizeof(sdp_pdu_hdr_t) )
					{
						/* find continuation state, if any
						*/
						if(save_and_cstate(rbuf,len))
						{
							notifier->flags( POLLOUT );

							state = SENDING;
						}
						else
						{
							notifier->flags( 0 );

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
			sess->on_read_response(ECONNABORTED, returned_records);
			break;
		}
		default:
			break;
	};
	sdp_dbg_leave();
	return;

sdp_error:

	u16 error = ntohs(*(u16*)(rbuf+sizeof(sdp_pdu_hdr_t)));
	sess->on_read_response(error, returned_records);

	sdp_dbg_leave();
}

void Client::Private::can_write( FdNotifier& )
{
	sdp_dbg_enter();

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
				memcpy(&sbuf,&obuf,sizeof(obuf));

				if(cstate.empty())
				{
					/* send null continuation state
					*/
					sbuf.data[sbuf.data_size] = '\0';
					sbuf.data_size += 1;
				}
				else
				{
					/* send previous continuation state
					*/
					memcpy(sbuf.data+sbuf.data_size, &cstate[0], cstate.size());
					sbuf.data_size += cstate.size();
				}

				/* calc req size & set headers
				*/
				req->tid = htons(__sdp_gen_tid());
				req->plen = htons(sbuf.data_size - sizeof(sdp_pdu_hdr_t));
			}

			int len = sock->send(sbuf.data+obytes, sbuf.data_size - obytes);
			if( len < 0 )
			{
				if( errno != EAGAIN && errno != EINTR )
				{
					state = DISCONNECTED;
					sdp_dbg("disconnected: %s", strerror(errno));
					goto w_disconn;
				}
			}
			else
			{
				obytes += len;
				if( obytes >= obuf.data_size )
				{
					state = WAITING;

					notifier->flags( POLLIN );
					obytes = 0;
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
	sdp_dbg_leave();
}

Client::Client()
:	pvt( new Private(this) )
{
	pvt->local = true;
}

Client::Client( const BdAddr& src, const BdAddr& dest )
:	pvt( new Private(this) )
{
	pvt->src = src;
	pvt->dst = dest;
	pvt->local = false;
}

Client::~Client()
{
sdp_dbg("end of session");
}

void Client::start_service_search( DataElementSeq& service_pattern )
{
	sdp_dbg_enter();

	if(
		pvt->state != Client::Private::IDLE &&
		pvt->state != Client::Private::DISCONNECTED
	)
		throw Error(EAGAIN);

	/* start writing header
	   (start_send will do the rest)
	*/
	pvt->req->pdu_id = SDP_SVC_SEARCH_REQ;

	/* append parameters
	*/

	/* start request
	*/
	pvt->service_pattern.pvt->list.swap(service_pattern.pvt->list);
	pvt->start_send();

	sdp_dbg_leave();
}

void Client::start_attribute_search( u32 service_handle, DataElementSeq& attributes )
{
	sdp_dbg_enter();

	if(
		pvt->state != Client::Private::IDLE &&
		pvt->state != Client::Private::DISCONNECTED
	)
		throw Error(EAGAIN);

	//sdp_data_t* attr_list = DataElement::Private::new_seq(attributes);
	sdp_data_t* attr_list = attributes.pvt->elem;

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

	pvt->req = (sdp_pdu_hdr_t*)pbuf->data;
	pvt->req->pdu_id = SDP_SVC_ATTR_REQ;

	//pvt->attribute_pattern = attributes;
	pvt->start_send();

	//free(attr_list);

	sdp_dbg_leave();
}

void Client::start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes )
{
	sdp_dbg_enter();

	if(
		pvt->state != Client::Private::IDLE &&
		pvt->state != Client::Private::DISCONNECTED
	)
		throw Error(EAGAIN);

//	sdp_data_t* svc_list = DataElement::Private::new_seq(service_pattern);
//	sdp_data_t* attr_list = DataElement::Private::new_seq(attributes);

	sdp_data_t* svc_list = service_pattern.pvt->elem;
	sdp_data_t* attr_list = attributes.pvt->elem;

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

	pvt->req = (sdp_pdu_hdr_t*)pbuf->data;
	pvt->req->pdu_id = SDP_SVC_SEARCH_ATTR_REQ;

	pvt->service_pattern.pvt->list.swap(service_pattern.pvt->list);

	pvt->start_send();

	sdp_dbg_leave();
}

/*	search for ALL attributes in ALL services
*/
void Client::start_complete_search()
{
	u16 public_group = PUBLIC_BROWSE_GROUP;

	DataElementSeq svcs;
	svcs.push_back( Sdp::UUID(public_group) );

	DataElementSeq attrs;
	attrs.push_back( Sdp::U32(0x0000FFFF) );

	start_attr_serv_search(svcs,attrs);
}

/*	cached search, nothing is transmitted/received over the air
*/

void Client::cached_complete_search()
{
	/*	this one is easy, just return everything
	*/
	this->on_read_response(0, pvt->cached_records);
}

}//namespace Sdp
