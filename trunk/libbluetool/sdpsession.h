#ifndef __SDP_SESSION_H
#define __SDP_SESSION_H

#include "../common/types.h"
#include "../common/fdnotifier.h"
#include "../common/timeout.h"

#include "l2csocket.h"

namespace Sdp
{
	class Session;
}

namespace Sdp
{

class Session
{
public:

	void start_service_search(  );

	void start_attribute_search(  );

	void start_attr_serv_search(  );

private:

	struct Private;
	Private* pvt;
};

}//namespace Sdp

#endif//__SDP_SESSION_H
