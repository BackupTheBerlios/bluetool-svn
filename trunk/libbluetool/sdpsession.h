#ifndef __SDP_SESSION_H
#define __SDP_SESSION_H

#include "../common/types.h"
#include "../common/fdnotifier.h"
#include "../common/timeout.h"
#include "../common/bytevector.h"
#include "../common/unixsocket.h"

namespace Sdp
{
	class Session;
}

#include "bdaddr.h"
#include "l2csocket.h"
#include "sdperror.h"
#include "sdprecord.h"

namespace Sdp
{

class Session
{
public:
	Session();

	Session( BdAddr& src, BdAddr& dest );

	~Session();

	void start_service_search( DataElementSeq& service_pattern );

	void start_attribute_search( u32 service_handle, DataElementSeq& attributes );

	void start_attr_serv_search( DataElementSeq& service_pattern, DataElementSeq& attributes );

	void start_complete_search();

private:

	SdpEvent on_response;

private:

	struct Private;
	Private* pvt;
};

}//namespace Sdp

#endif//__SDP_SESSION_H
