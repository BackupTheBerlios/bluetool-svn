#ifndef __BTOOL_SDP_BROWSER_H
#define __BTOOL_SDP_BROWSER_H

#include <cbus/cbus.h>
#include <bluetool/sdprecord.h>
#include <bluetool/sdpclient.h>
#include <common/refptr.h>
#include "../btool_names.h"

namespace Bluetool
{
	class SdpBrowser;
}

//#include "../btool_device.h"
//#include "../hci/hci_device.h"
#include "sdp_record.h"

namespace Bluetool
{

class SdpBrowser : public DBus::LocalInterface, public Sdp::Client
{
public:
	SdpBrowser( const BdAddr& from, const BdAddr& to );

	~SdpBrowser();

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
	DBus::Connection _bus;
	DBus::ReturnMessage* _reply;

	SdpRecordPTable _records;
};

}//namespace Bluetool

#endif//__BTOOL_SDP_BROWSER_H
