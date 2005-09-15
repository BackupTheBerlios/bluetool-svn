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


#include <dbus/dbus.h>

#include <cbus/cbusmessage.h>

namespace DBus
{

Message::Message()
{}

Message::Message( DBusMessage* m )
{
	_message = m;
	ref();
}

Message::Message( const Message& m )
{
	_message = m._message;
	ref();
}

Message::~Message()
{
	unref();
}

Message Message::copy()
{
	return Message(dbus_message_copy(_message));
}

ErrorMessage::ErrorMessage()
{
	_message = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
}

ErrorMessage::ErrorMessage( const Message& to_reply, const char* name, const char* message )
{
	_message = dbus_message_new_error(to_reply._message, name, message);
}

bool ErrorMessage::operator == ( const ErrorMessage& m ) const
{
	return dbus_message_is_error(_message, m.name());
}

SignalMessage::SignalMessage( const char* name )
{
	_message = dbus_message_new(DBUS_MESSAGE_TYPE_SIGNAL);
	member(name);
}

SignalMessage::SignalMessage( const char* path, const char* interface, const char* name )
{
	_message = dbus_message_new_signal(path, interface, name);
}

bool SignalMessage::operator == ( const SignalMessage& m ) const
{
	return dbus_message_is_signal(_message, m.interface(), m.member());
}

CallMessage::CallMessage( const char* dest, const char* path, const char* iface, const char* method )
{
	_message = dbus_message_new_method_call(dest, path, iface, method);
}

bool CallMessage::operator == ( const CallMessage& m ) const
{
	return dbus_message_is_method_call(_message, m.interface(), m.member());
}

ReturnMessage::ReturnMessage( const CallMessage& callee )
{
	_message = dbus_message_new_method_return(callee._message);
}

bool Message::append( int first_type, ... )
{
	va_list vl;
	va_start(vl, first_type);

	bool b = dbus_message_append_args_valist(_message, first_type, vl);

	va_end(vl);
	return b;
}

void Message::terminate()
{
	dbus_message_append_args(_message,DBUS_TYPE_INVALID);
}

}//namespace DBus
