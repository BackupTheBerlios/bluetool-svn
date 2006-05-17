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
#include <cbus/cbusobject.h>

namespace DBus
{

Object::Object( const char* name, Connection& conn )
:	_conn(conn), _name(name)
{
}

Object::~Object()
{
}

const Object* Object::object() const
{
	return this;
}

LocalObject::LocalObject( const char* name, Connection& conn )
:	Object(name,conn)
{
	register_obj();
}

LocalObject::~LocalObject()
{
	if(one()) unregister_obj();
}

DBusObjectPathVTable LocalObject::_vtable =
{	
	unregister_function_stub,
	message_function_stub,
	NULL, NULL, NULL, NULL
};

void LocalObject::register_obj()
{
	cbus_dbg("registering local object %s", oname().c_str());
	if(!dbus_connection_register_object_path(conn()._connection, oname().c_str(), &_vtable, this))
	{
		cbus_dbg("error registering object path %s", oname().c_str());		
 		//throw Error(NULL,"Unable to register object");
	}
	else
	{
		InterfaceTable::const_iterator ii = _interfaces.begin();
		while( ii != _interfaces.end() )
		{
			std::string im = "type='method_call',interface='"+ii->first+"'";
			conn().add_match(im.c_str());
			++ii;
		}
	}
}

void LocalObject::unregister_obj()
{
	cbus_dbg("unregistering local object %s", oname().c_str());
	dbus_connection_unregister_object_path(conn()._connection, oname().c_str());

	InterfaceTable::const_iterator ii = _interfaces.begin();
	while( ii != _interfaces.end() )
	{
		std::string im = "type='method_call',interface='"+ii->first+"'";
		conn().remove_match(im.c_str());
		++ii;
	}
}

void LocalObject::unregister_function_stub( DBusConnection* conn, void* data )
{
 	// XXX: what do we have to do here ?
}

DBusHandlerResult LocalObject::message_function_stub( DBusConnection*, DBusMessage* dmsg, void* data )
{
	Message msg(dmsg);	

	LocalObject* o = static_cast<LocalObject*>(data);

	if( o )
	{
		cbus_dbg("got message #%d from %s to %s (in %s)",
			msg.serial(), 
			msg.sender(), 
			msg.destination(),
			o->oname().c_str()
		);

		return o->handle_message(msg) ? DBUS_HANDLER_RESULT_HANDLED : DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
	else return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void LocalObject::remit_signal( SignalMessage& sig )
{
	sig.path(oname().c_str());

	conn().send(sig);
}

bool LocalObject::handle_message( const Message& msg )
{
	switch( msg.type() )
	{
		case DBUS_MESSAGE_TYPE_METHOD_CALL:
		{
			const CallMessage& cmsg = reinterpret_cast<const CallMessage&>(msg);
			const char* interface	= cmsg.interface();
		
			InterfaceTable::const_iterator ii = _interfaces.find(interface);
			if( ii != _interfaces.end() )
			{
				return ii->second->invoke_method(cmsg);
			}
			else	return false;
		}
		default:	return false;
	}
}

RemoteObject::RemoteObject( const char* name, Connection& conn )
:	Object(name, conn)
{
	register_obj();
}

#if 0
RemoteObject::RemoteObject( const RemoteObject& )
{
	_receiver.filtered.connect( sigc::mem_fun( this, &RemoteObject::handle_message ) );
	conn().add_filter(_receiver);
}
#endif

RemoteObject::~RemoteObject()
{
	if(noref()) unregister_obj();
}

void RemoteObject::register_obj()
{
	cbus_dbg("registering remote object %s", oname().c_str());
	_receiver.filtered.connect( sigc::mem_fun( this, &RemoteObject::handle_message ) );
	
	conn().add_filter(_receiver);

	InterfaceTable::const_iterator ii = _interfaces.begin();
	while( ii != _interfaces.end() )
	{
		std::string im = "type='signal',interface='"+ii->first+"'";
		conn().add_match(im.c_str());
		++ii;
	}
	
//	conn().add_match("type='signal'");
//	conn().add_match("type='method_call'");
}

void RemoteObject::unregister_obj()
{
	cbus_dbg("unregistering remote object %s", oname().c_str());
	InterfaceTable::const_iterator ii = _interfaces.begin();
	while( ii != _interfaces.end() )
	{
		std::string im = "type='signal',interface='"+ii->first+"'";
		conn().remove_match(im.c_str());
		++ii;
	}
//	conn().remove_match("type='method_call'");
//	conn().remove_match("type='signal'");

	conn().remove_filter(_receiver);
}

bool RemoteObject::handle_message( const Message& msg )
{
	switch( msg.type() )
	{
		case DBUS_MESSAGE_TYPE_SIGNAL:
		{
			const SignalMessage& smsg = reinterpret_cast<const SignalMessage&>(msg);
			const char* interface	= smsg.interface();
			const char* member	= smsg.member();

			cbus_dbg("filtered signal %s(in %s) from remote object %s", member, interface, msg.sender());

			InterfaceTable::const_iterator ii = _interfaces.find(interface);
			if( ii != _interfaces.end() )
			{
				return ii->second->dispatch_signal(smsg);
			}
			else	return false;
		}
		case DBUS_MESSAGE_TYPE_METHOD_RETURN:
		{
			cbus_dbg("filtered method return from remote object %s", msg.sender());

			//TODO:
			return false;
		}
		case DBUS_MESSAGE_TYPE_ERROR:
		{
			cbus_dbg("filtered error from remote object %s", msg.sender());

			//TODO:
			
			return false;
		}
		default:	return false;
	}
}

}//namespace DBus
