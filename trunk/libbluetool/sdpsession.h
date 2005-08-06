#ifndef __SDP_SESSION_H
#define __SDP_SESSION_H

#include "../common/types.h"
#include "../common/fdnotifier.h"
#include "../common/timeout.h"
#include "../common/bytevector.h"

#include "bdaddr.h"
#include "l2csocket.h"
#include "sdperror.h"
#include "sdprecord.h"

namespace Sdp
{
	class Session;
}

namespace Sdp
{

class Session
{
public:

	Session( BdAddr& src, BdAddr& dest );

	void start_service_search( DataElementList& service_pattern );

	void start_attribute_search( u32 service_handle, DataElementList& attributes );

	void start_attr_serv_search( DataElementList& service_pattern, DataElementList& attributes );

private:

	struct Private;
	Private* pvt;
};

}//namespace Sdp

#endif//__SDP_SESSION_H
