#include <dbus/dbus.h>

#include "cbusmessage.h"

namespace DBus
{

Message::Message()
{//todo
}

Message::Message( DBusMessage* m )
{
	_message = dbus_message_copy(m);
}

Message::Message( const Message& m )
{
	_message = dbus_message_copy(m._message);
}

Message::~Message()
{
	unref();
}

ErrorMessage::ErrorMessage( Message& to_reply, const char* name, const char* message )
{
	_message = dbus_message_new_error(to_reply._message, name, message);
}

bool ErrorMessage::operator == ( const ErrorMessage& m ) const
{
	return dbus_message_is_error(_message, m.name());
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

/*MessageIter::MessageIter( Message& m )
{
	if(!dbus_message_iter_init(m._message, &_iter))
		throw Error("FormatError", "Cannot iterate arguments");//todo
}*/
/*
bool Message::read( const char* format, int first_type, ... ) const
{
	Error e;

	va_list vl;
	va_start(vl, first_type);
	va_end(vl);

	bool b = dbus_message_get_args_valist(_message, e, first_type, vl);
	if(e)	throw e;
	return b;
}

bool Message::write( const char* format, int first_type, ... )
{
	va_list vl;
	va_start(vl, first_type);
	va_end(vl);

	return dbus_message_append_args_valist(_message, first_type, vl);
}
*/
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
