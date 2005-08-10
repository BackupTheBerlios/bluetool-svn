#ifndef __BTOOL_HCI_SERVICE_H
#define __BTOOL_HCI_SERVICE_H

#include "../../cbus/cbus.h"
#include "../../common/eventloop.h"

#include "../btool_common.h"

#include "hci_tracker.h"
#include "hci_device.h"

class HciManager : public DBus::LocalInterface, public DBus::LocalObject
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

	HciDevice* get_device( const char* name );

private:
	HciTracker _tracker;
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
