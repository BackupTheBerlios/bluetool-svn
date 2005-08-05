#include "hci_manager.h"

#include <iostream>
using namespace std;

HciService::HciService()
:	_dbus_conn( DBus::Connection::SystemBus() ),
	
 	_hci_manager( _dbus_conn )
{
	/*	register HCI service
	*/
	_dbus_conn.request_name( DBUS_HCI_SERVICE );
}

/*
*/

HciManager::HciManager( DBus::Connection& conn )
:	DBus::LocalObject	( DBUS_HCIMAN_PATH, conn ),
	DBus::LocalInterface	( DBUS_HCIMAN_IFACE )
	//_tracker		( _devices )
{
	/*	export all methods in the interface
	*/
	register_method( HciManager, ListDevices );
	register_method( HciManager, EnableDevice );
	register_method( HciManager, DisableDevice );
	register_method( HciManager, ResetDevice );
}

void HciManager::ListDevices( const DBus::CallMessage& msg )
{
}

void HciManager::EnableDevice( const DBus::CallMessage& msg )
{
}

void HciManager::DisableDevice( const DBus::CallMessage& msg )
{
}

void HciManager::ResetDevice( const DBus::CallMessage& msg )
{
}


HciDevice* HciManager::get_device( int dev_id )
{
/*	HciDevicePTable::iterator i = _devices.find(dev_id);
	if( i != _devices.end() )
	{
		return i->second;	
	}
*/	return NULL;
}
