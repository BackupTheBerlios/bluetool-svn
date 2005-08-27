#ifndef __BTOOL_DEVICE_H
#define __BTOOL_DEVICE_H

#include <map>
#include <common/refptr.h>
#include <bluetool/bdaddr.h>
#include <cbus/cbus.h>

#include "btool_names.h"

namespace Bluetool
{
	class Device;
	typedef std::map <int, RefPtr<Device> >	DeviceRTable;

	class RemoteDevice;
}

#include "hci/hci_device.h"
#include "sdp/sdp_browser.h"
#include "services/btool_service_database.h"

namespace Bluetool
{

class Device : public HciDevice, public DBus::LocalObject
{
public:
	Device( int dev_id, const BdAddr& );
	~Device();

	Hci::RemoteDevice* on_new_cache_entry( Hci::RemoteInfo& );

private:
	ServiceDatabase	_services;
};

class RemoteDevice : public HciRemote, public SdpBrowser, public DBus::LocalObject
{
public:
	RemoteDevice( Device*, Hci::RemoteInfo& );
	~RemoteDevice();

	Hci::Connection* on_new_connection( Hci::ConnInfo& );

private:
	Device* _parent;
};

class Connection : public HciConnection, public DBus::LocalObject
{
public:
	Connection( RemoteDevice*, Hci::ConnInfo& );
};

}//namespace Bluetool

#endif//__BTOOL_DEVICE_H
