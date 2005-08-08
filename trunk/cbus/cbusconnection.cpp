#include "cbusdebug.h"

#include <algorithm>	//for find

#include "cbusconnection.h"

namespace DBus
{

static Connection* g_system_bus = NULL;

Connection& Connection::SystemBus()
{
	//return Connection(DBUS_BUS_SYSTEM);	//TODO: allow to safely make a copy of a connection, 'cause this sucks!
	if(!g_system_bus)
		g_system_bus = new Connection(DBUS_BUS_SYSTEM);

	return *g_system_bus;
}

Connection Connection::SessionBus()
{
	return Connection(DBUS_BUS_SESSION);
}

Connection::Connection( DBusBusType type )
{
	Error e;
	_connection = dbus_bus_get(type,e);

	if(e) throw e;

	//_debug_filter.filtered.connect( sigc::mem_fun(*this, &Connection::_debug_filter_function) );

	//add_filter(_debug_filter);

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
}

void Connection::init()
{
	Monitor::init(_connection);

	this->_dispatch_pending = true;

	const char* service = this->unique_name();
	if(service)
	{
		string match = string("destination='") + string(service) + string("'");
		this->add_match(match.c_str());
	}

	//dbus_connection_add_filter(_connection, /*filter_handler*/NULL, this, 0); //FIXME

	dbus_connection_set_dispatch_status_function(_connection, dispatch_status_stub, this, 0);
	dbus_connection_set_exit_on_disconnect(_connection, false);
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
//	return dbus_connection_send(_connection, msg._message, serial);
	if(dbus_connection_send(_connection, msg._message, serial))
	{
		//_dispatch_pending = true;
		//do_dispatch();
		//dbus_connection_flush(_connection);
		return true;
	}
	return false;
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
		string match = "destination='" + _service_name + "'";
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
#if 0
DBusObjectPathVTable Connection::_vtable =
{	
	unregister_function_stub,
	message_function_stub,
	NULL, NULL, NULL, NULL
};

bool Connection::register_object( LocalObject* o )
{
	cbus_dbg("%s: registering local object %s",unique_name(),o->name().c_str());

	if(dbus_connection_register_object_path(_connection, o->name().c_str(), &_vtable, o))
	{
		_registered_objects.push_back(o);
		return true;
	}
	else	return false;
}

bool Connection::unregister_object( LocalObject* o )
{
	cbus_dbg("%s: unregistering local object %s",unique_name(),o->name().c_str());

	ObjectPList::iterator oi = std::find(
		_registered_objects.begin(),
		_registered_objects.end(),
		o
	);
	if( oi != _registered_objects.end() && (*oi)->name() == o->name() )
	{	
		dbus_connection_unregister_object_path(_connection, o->name().c_str());
		return true;
	}
	return false;
}

bool Connection::register_object( RemoteObject* o )
{
	cbus_dbg("%s: registering remote object %s",unique_name(),o->name().c_str());

	if(add_filter( o->_receiver ))
	{
//		string match = "interface='" + o->name() + "'";
//		add_match(match.c_str());
		_registered_objects.push_back(o);
		return true;
	}
	return false;
}

bool Connection::unregister_object( RemoteObject* o )
{
	cbus_dbg("%s: unregistering remote object %s",unique_name(),o->name().c_str());

	ObjectPList::iterator oi = std::find(
		_registered_objects.begin(),
		_registered_objects.end(),
		o
	);
	if( oi != _registered_objects.end() && (*oi)->name() == o->name() )
	{	
	//	remove_match("type='signal'");

	
	//	string match = "source='" + o->name() + "'";
	//	remove_match(match.c_str());

		o->_base_service = NULL;
		remove_filter( o->_receiver );
	
	
 	}

	return true;
}

void Connection::unregister_function_stub( DBusConnection* conn, void* data )
{
	LocalObject* o = static_cast<LocalObject*>(data);
	Connection* c = o->_base_service;
	if(c)
	{
		ObjectPList::iterator oi = std::find(
			c->_registered_objects.begin(),
			c->_registered_objects.end(),
			o
		);
		if( oi != c->_registered_objects.end() )
		{
			(*oi)->_base_service = NULL;
			c->_registered_objects.erase(oi);
		}
	}
	//else error
}

DBusHandlerResult Connection::message_function_stub( DBusConnection*, DBusMessage* dmsg, void* data )
{
	Message msg(dmsg);	

	Object* o = static_cast<Object*>(data);

	if( o )
	{
		cbus_dbg("got message from %s for object %s", msg.destination(), o->name().c_str());

		return o->handle_message(msg) ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	else return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
/*
00042 typedef enum
00043 {
00044   DBUS_HANDLER_RESULT_HANDLED,         
00045   DBUS_HANDLER_RESULT_NOT_YET_HANDLED, 
00046   DBUS_HANDLER_RESULT_NEED_MEMORY      
00047 } DBusHandlerResult;
*/
}
#endif

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

/*
00086 typedef DBusHandlerResult (* DBusHandleMessageFunction) (DBusConnection     *connection,
00087                                                          DBusMessage        *message,
00088                                                          void               *user_data);
*/

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

}//namespace DBus
