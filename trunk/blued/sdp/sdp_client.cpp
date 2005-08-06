#include "../bluedebug.h"
#include "sdp_client.h"

SdpClient::SdpClient()
:	DBus::LocalInterface( SDP_CLIENT_IFACE ),
	DBus::LocalObject( SDP_CLIENT_PATH, DBus::Connection::SystemBus() )
{
	register_method( SdpClient, SearchServices );
	register_method( SdpClient, SearchAttributes );
	register_method( SdpClient, SearchServAttrs );
}


void SdpClient::SearchServices( const DBus::CallMessage& )
{
}

void SdpClient::SearchAttributes( const DBus::CallMessage& )
{
}

void SdpClient::SearchServAttrs( const DBus::CallMessage& )
{
}
