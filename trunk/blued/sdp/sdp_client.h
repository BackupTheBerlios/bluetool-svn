#ifndef __SDP_CLIENT_H
#define __SDP_CLIENT_H

#include "../../common/timeout.h"
#include "../../common/fdnotifier.h"
#include "../../cbus/cbus.h"
#include "../../libbluetool/sdpsession.h"

#define SDP_CLIENT_PATH		"/org/bluetool/sdp/client"
#define SDP_CLIENT_IFACE	"org.bluetool.sdp.client"

class SdpClient : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	SdpClient();

public:
	/*	exported methods
	*/
	void SearchServices( const DBus::CallMessage& );

	void SearchAttributes( const DBus::CallMessage& );

	void SearchServAttrs( const DBus::CallMessage& );
};

#endif//__SDP_CLIENT_H
