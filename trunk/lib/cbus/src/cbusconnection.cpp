/*
 *
 *  C-Bus - C++ bindings for DBus
 *
 *  Copyright (C) 2005-2006  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <cbus/cbusdebug.h>

#include <algorithm>	//for find

#include <cbus/cbusconnection.h>

using namespace DBus;

static Connection* g_system_bus = NULL;
static Connection* g_activation_bus = NULL;	//TODO: that's a BAAD thing

Connection& Connection::SystemBus()
{
	//return Connection(DBUS_BUS_SYSTEM);
	if(!g_system_bus)
		g_system_bus = new Connection(DBUS_BUS_SYSTEM);

	return *g_system_bus;
}

Connection Connection::SessionBus()
{
	return Connection(DBUS_BUS_SESSION);
}

Connection& Connection::ActivationBus()
{
	if(!g_activation_bus)
		g_activation_bus = new Connection(DBUS_BUS_STARTER);

	return *g_activation_bus;
}

Connection::Connection( DBusBusType type )
{
	Error e;
	_connection = dbus_bus_get(type,e);

	if(e) throw e;

	_disconn_filter.filtered.connect( sigc::mem_fun(*this, &Connection::_disconn_filter_function) );

	add_filter(_disconn_filter);

	init();
}

Connection::Connection( const char* address, bool priv )
{
	Error e;
	_connection = priv 
			? NULL//dbus_connection_open_private(address, e) //todo:
			: dbus_connection_open(address,e);

	if(e) throw e;

	init();

	cbus_dbg("connected to %s", address);
}

Connection::Connection( DBusConnection* c ) 
:	_connection(c)
{
	init();
	ref();
}

void Connection::init()
{
	Monitor::init(_connection);

	this->_dispatch_pending = true;

	const char* service = this->unique_name();
	if(service)
	{
		std::string match = 
			  std::string("destination='")
			+ std::string(service)
			+ std::string("'");

		this->add_match(match.c_str());
	}

	//dbus_connection_add_filter(_connection, /*filter_handler*/NULL, this, 0); //FIXME

	dbus_connection_set_dispatch_status_function(_connection, dispatch_status_stub, this, 0);
	dbus_connection_set_exit_on_disconnect(_connection, true);
}

Connection::Connection( const Connection& c )
:	Monitor(c), _connection(c._connection), _service_name(c._service_name)
{
	ref();
}

Connection::~Connection()
{
	unref();
}

void Connection::add_match( const char* rule )
{
	Error e;

	dbus_bus_add_match(_connection, rule, e);

	cbus_dbg("%s: added match rule %s", unique_name(), rule);

	if(e) throw e;
}

void Connection::remove_match( const char* rule )
{
	Error e;
	
	dbus_bus_remove_match(_connection, rule, e);

	cbus_dbg("%s: removed match rule %s", unique_name(), rule);

	if(e) throw e;
}

bool Connection::add_filter( Filter& f )
{
	cbus_dbg("%s: adding filter",unique_name());
	return dbus_connection_add_filter(_connection, message_filter_stub, &f, NULL);
}

void Connection::remove_filter( Filter& f )
{
	cbus_dbg("%s: removing filter",unique_name());
	dbus_connection_remove_filter(_connection, message_filter_stub, &f);
}

bool Connection::send( const Message& msg, unsigned int* serial )
{
	return dbus_connection_send(_connection, msg._message, serial);
}

Message Connection::send_blocking( Message& msg, int timeout )
{
	DBusMessage* reply;
	Error e;
	reply = dbus_connection_send_with_reply_and_block(_connection, msg._message, timeout, e);
	if(e) throw e;

	return Message(reply);
}

void Connection::request_name( const char* name, int flags )
{
	Error e;

	cbus_dbg("%s: registering bus name %s", unique_name(), name);

#ifdef DBUS_LEGACY
	dbus_bus_acquire_service(_connection, name, flags, e);
#else
	dbus_bus_request_name(_connection, name, flags, e);	//we deliberalely don't check return value
#endif

	if(e)	throw e;

//	this->remove_match("destination");

	if(name)
	{
		this->_service_name = name;
		std::string match = "destination='" + _service_name + "'";
		this->add_match(match.c_str());
	}
}

bool Connection::has_name( const char* name )
{	
	Error e;

#ifdef DBUS_LEGACY
	bool b = dbus_bus_service_exists(_connection, name, e);
#else
	bool b = dbus_bus_name_has_owner(_connection, name, e);
#endif

	if(e) throw e;	//hmmm, do we need that ??
	return b;
}

bool Connection::start_service( const char* name, u32 flags )
{
	Error e;

	bool b = dbus_bus_start_service_by_name(_connection,name,flags,NULL,e);

	if(e) throw e;
	return b;
}

void Connection::do_dispatch()
{
	if(_dispatch_pending)
//	if( dbus_connection_get_dispatch_status(_connection) == DBUS_DISPATCH_DATA_REMAINS)
	{
		cbus_dbg("dispatching on %p", _connection);

		while(dbus_connection_dispatch(_connection) == DBUS_DISPATCH_DATA_REMAINS) 
		; //loop
		_dispatch_pending = false;
	}

}

void Connection::dispatch_status_stub( DBusConnection* dc, DBusDispatchStatus status, void* data )
{
	Connection* c = static_cast<Connection*>(data);

	switch( status )
	{
		case	DBUS_DISPATCH_DATA_REMAINS:
		cbus_dbg("some dispatching to do on %p", dc);
		c->_dispatch_pending = true;	//tells the monitor this connection needs dispatching
		break;

		case 	DBUS_DISPATCH_COMPLETE:
		cbus_dbg("all dispatching done on %p", dc);
		break;

		case	DBUS_DISPATCH_NEED_MEMORY: //uh oh...
		cbus_dbg("connection %p needs memory", dc);
		break;
	}
}

DBusHandlerResult Connection::message_filter_stub( DBusConnection*, DBusMessage* dmsg, void* data )
{
	Filter* f = static_cast<Filter*>(data);

	Message msg = Message(dmsg);

	/*	TODO: how to translate a DBusConnection into a Connection ?
	*/
	return f && !f->filtered.empty() && f->filtered(msg) 
			? DBUS_HANDLER_RESULT_HANDLED
			: DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

bool Connection::_disconn_filter_function( Message& msg )
{
	if(msg.is_signal(DBUS_INTERFACE_LOCAL,"Disconnected"))
	{
		cbus_dbg("%p disconnected by local bus",_connection);
		disconnect();
		return true;
	}
	return false;
}
