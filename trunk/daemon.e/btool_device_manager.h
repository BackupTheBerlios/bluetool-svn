#ifndef __BTOOL_HCI_SERVICE_H
#define __BTOOL_HCI_SERVICE_H

#include "../libbluetool/hcisocket.h"
#include "../cbus/cbus.h"
#include "../common/fdnotifier.h"

#include "btool_names.h"
#include "btool_device.h"

namespace Bluetool
{

class DeviceManager : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	DeviceManager();

	/*	public methods
	*/
	void ListDevices	( const DBus::CallMessage& );
	void EnableDevice	( const DBus::CallMessage& );
	void DisableDevice	( const DBus::CallMessage& );
	void ResetDevice	( const DBus::CallMessage& );

private:

	/*	signal emitters
	*/
	void DeviceAdded( const char* name );
	void DeviceRemoved( const char* name );

private:

	Device* get_device( int );

	void on_new_event( FdNotifier& );

private:

	FdNotifier	_notifier;
	Hci::Socket	_evt_socket;
	DevicePTable	_devices;
};

}//namespace Bluetool

#endif//__BTOOL_HCI_SERVICE_H
