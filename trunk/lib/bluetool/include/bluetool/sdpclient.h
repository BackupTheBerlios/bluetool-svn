#ifndef __SDP_CLIENT_H
#define __SDP_CLIENT_H

#include <common/types.h>
#include <common/fdnotifier.h>
#include <common/timeout.h>
#include <common/bytevector.h>
#include <common/unixsocket.h>
#include <common/refptr.h>

namespace Sdp
{
	class Client;
}

#include "bdaddr.h"
#include "l2csocket.h"
#include "sdperror.h"
#include "sdprecord.h"

namespace Sdp
{

class Client
{
public:
	Client();

	Client( const BdAddr& src, const BdAddr& dest );

	virtual ~Client();

	/*
	*/

	void start_service_search( DataElementSeq& service_pattern );

	void start_attribute_search( u32 service_handle, DataElementSeq& attributes );

	void start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes );

	void start_complete_search();

	/*
	*/

//	RecordList& cached_service_search( DataElementSeq& service_pattern );

//	RecordList& cached_attribute_search( u32 service_handle, DataElementSeq& attributes );

//	RecordList& cached_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes );

	void cached_complete_search();

	/*
	*/

private:

	virtual void on_read_response( u16 error, const RecordList& records ) = 0;

	virtual void on_new_cache_entry( Record& ) = 0;

	virtual void on_purge_cache_entry( Record& ) = 0;

public:

	//SdpEvent on_response; // todo: a virtual method is enough here

private:

	struct Private;
	RefPtr<Private> pvt;
};

}//namespace Sdp

#endif//__SDP_CLIENT_H
