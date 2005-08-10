#ifndef __CBUS_CONNECTION_H
#define __CBUS_CONNECTION_H

#include <list>
#include <dbus/dbus.h>

namespace DBus
{
class Connection;

typedef std::list<Connection*>	ConnectionPList;
}

#include "cbusmonitor.h"
#include "cbuserror.h"
#include "cbusmessage.h"
#include "cbuspendingcall.h"
//#include "cbusobject.h"
#include "cbusfilter.h"

namespace DBus
{
//friends
//class LocalObject;
//class RemoteObject;

class Connection : public Monitor
{
public:

	static Connection& SystemBus();

	static Connection SessionBus();

	Connection( DBusBusType bustype );

	Connection( const char* address, bool priv = false );

	Connection( DBusConnection* c );

	Connection( const Connection& c );

	~Connection();

	inline bool operator == ( const Connection& ) const;

	void add_match( const char* rule );

	void remove_match( const char* rule );

	bool add_filter( Filter& );

	void remove_filter( Filter& );

//	inline bool unique_name( const char* n );

	inline const char* unique_name() const;

	inline bool connected() const;

	inline void disconnect();

	inline void flush();

	bool send( const Message&, unsigned int* serial = NULL );

	Message send_blocking( Message& msg, int timeout );

	void request_name( const char* name, int flags = 0 );

	bool has_name( const char* name );

private:

	inline void ref();

	inline void unref();

	void init();

	static void dispatch_status_stub( DBusConnection*, DBusDispatchStatus, void* );

	static DBusHandlerResult message_filter_stub( DBusConnection*, DBusMessage*, void* );

public:

	DBusConnection 	*_connection;

private:

	virtual void do_dispatch();

private:

	bool _dispatch_pending;

//	ObjectPList	_registered_objects;

	Filter		_disconn_filter;
	bool		_disconn_filter_function( Message& );
//	Filter		_debug_filter;
//	bool		_debug_filter_function( Message& );

	std::string	_service_name;

//friend class LocalObject;
//friend class RemoteObject;
};

/*
*/

bool Connection::operator == ( const Connection& c ) const
{
	return _connection == c._connection;
}
bool Connection::connected() const
{
	return dbus_connection_get_is_connected(_connection);
}
void Connection::disconnect()
{
	dbus_connection_disconnect(_connection);
}
#if 0
bool Connection::unique_name( const char* n )
{
	return dbus_bus_set_unique_name(_connection, n);
}
#endif
const char* Connection::unique_name() const
{
#ifdef DBUS_LEGACY
	return dbus_bus_get_base_service(_connection);
#else
	return dbus_bus_get_unique_name(_connection);
#endif

}
void Connection::flush()
{
	dbus_connection_flush(_connection);
}
void Connection::ref()
{
	dbus_connection_ref(_connection);
}
void Connection::unref()
{
	dbus_connection_unref(_connection);
}

}//namespace DBus

#endif//__CBUS_CONNECTION_H
