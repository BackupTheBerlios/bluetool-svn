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
:	DBus::LocalInterface	( DBUS_HCIMAN_IFACE ),
	DBus::LocalObject	( DBUS_HCIMAN_PATH, conn )
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
	DBus::ReturnMessage reply (msg);
	DBus::MessageIter rw = reply.w_iter();

	HciDevicePTable::iterator i = _tracker._devices.begin();
	while( i != _tracker._devices.end() )
	{
		const char* fullname = i->second->oname().c_str();
		rw.append_string(fullname);
		++i;
	}
	reply.append(DBUS_TYPE_INVALID);
	conn().send(reply);
}

void HciManager::EnableDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		DBus::ReturnMessage reply (msg);

		Hci::LocalDevice::up(name);

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}

void HciManager::DisableDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		DBus::ReturnMessage reply (msg);

		Hci::LocalDevice::down(name);

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}

void HciManager::ResetDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		Hci::LocalDevice::reset(name);

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}


HciDevice* HciManager::get_device( const char* name )
{
	HciDevicePTable::iterator i = _tracker._devices.find(name);
	if( i != _tracker._devices.end() )
	{
		return i->second;	
	}
	return NULL;
}
