#ifndef __BTOOL_HCI_SERVICE_H
#define __BTOOL_HCI_SERVICE_H

#include "../../cbus/cbus.h"
#include "../../common/eventloop.h"

#include "../dbus_names.h"

#include "hci_tracker.h"
#include "hci_device.h"

/*
	basically, that's what we see from the system bus:

	org.bluetool.hci		the service containing all our bus objects

	org.bluetool.hci.manager	the interface which specifies the behaviour of the HciManager,
					this interface consists of

	DeviceAdded			signal
	DeviceRemoved			signal

	EnumDevices			method

	/org/bluetool/hci/manager	the actual object implementing the above interface

	org.bluetool.hci.device		the interface representing a device

	/org/bluetool/hci/hciX		(where X is a small integer) the object devices
*/

class HciManager : public DBus::LocalObject, public DBus::LocalInterface
{
public:
	HciManager( DBus::Connection& conn );

	/*	public methods
	*/

	void ListDevices	( const DBus::CallMessage& );
	void EnableDevice	( const DBus::CallMessage& );
	void DisableDevice	( const DBus::CallMessage& );
	void ResetDevice	( const DBus::CallMessage& );

private:

	/*	signal emitters
	*/

	void HciDeviceAdded( const char* name );
	void HciDeviceRemoved( const char* name );

private:

	HciDevice* get_device( int dev_id );

private:
	HciTracker _tracker;
	//HciDevicePTable	_devices;
};

class HciService
{
public:
	HciService();
private:
	DBus::Connection	_dbus_conn;
	HciManager		_hci_manager;
};

#endif//__BTOOL_HCI_SERVICE_H
