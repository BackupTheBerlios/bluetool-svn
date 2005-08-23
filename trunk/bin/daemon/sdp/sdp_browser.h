#ifndef __BTOOL_SDP_BROWSER_H
#define __BTOOL_SDP_BROWSER_H

#include <cbus/cbus.h>
#include <bluetool/sdpsession.h>
#include <bluetool/hcidevice.h>

class SdpBrowser : public DBus::LocalInterface
{
public:
	SdpBrowser( Hci::LocalDevice* from, const BdAddr& to );

public:
	/*	exported methods
	*/
	void SearchServices( const DBus::CallMessage& );

	void SearchAttributes( const DBus::CallMessage& );

	void SearchServAttrs( const DBus::CallMessage& );

	void SearchAllRecords( const DBus::CallMessage& );

private:
	void on_read_response( u16 status, const Sdp::RecordList& data );

private:
	Hci::LocalDevice* _from;
	Sdp::Session _session;
	DBus::Connection _bus;

	DBus::ReturnMessage* _reply;
};

#endif//__BTOOL_SDP_BROWSER_H
