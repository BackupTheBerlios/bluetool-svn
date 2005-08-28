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

	~Client();

	void start_service_search( DataElementSeq& service_pattern );

	void start_attribute_search( u32 service_handle, DataElementSeq& attributes );

	void start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes );

	void start_complete_search();

public:

	SdpEvent on_response;

private:

	struct Private;
	RefPtr<Private> pvt;
};

}//namespace Sdp

#endif//__SDP_CLIENT_H
