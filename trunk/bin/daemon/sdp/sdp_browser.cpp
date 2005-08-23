#include "sdp_browser.h"

#include "../btool_names.h"

SdpBrowser::SdpBrowser( Hci::LocalDevice* from, const BdAddr& to )
:	DBus::LocalInterface(BTOOL_SDPBROWSER_IFACE),
	_from(from),
	_session(from->addr(), to),
	_bus(DBus::Connection::SystemBus()),
	_reply(NULL)
{
	register_method( SdpBrowser, SearchServices );
	register_method( SdpBrowser, SearchAttributes );
	register_method( SdpBrowser, SearchServAttrs );
	register_method( SdpBrowser, SearchAllRecords );

	_session.on_response.connect( sigc::mem_fun( this, &SdpBrowser::on_read_response ));
}

void SdpBrowser::SearchServices( const DBus::CallMessage& )
{
}

void SdpBrowser::SearchAttributes( const DBus::CallMessage& )
{
}

void SdpBrowser::SearchServAttrs( const DBus::CallMessage& )
{
}

void SdpBrowser::SearchAllRecords( const DBus::CallMessage& )
{
}

void SdpBrowser::on_read_response( u16 status, const Sdp::RecordList& data )
{

	_bus.send(*_reply);
	delete _reply;
	_reply = NULL;
}
