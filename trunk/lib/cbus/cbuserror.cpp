#include "cbusdebug.h"
#include "cbusmessage.h"
#include "cbuserror.h"

namespace DBus
{

Error::Error()
{
	dbus_error_init(&_error);
}

Error::Error( DBusError* error )
{
	dbus_move_error(error, &_error);

#ifdef _DEBUG
	if( is_set() ) cbus_dbg("error %s: %s",name(), message() );
#endif
}

Error::Error( const char* name, const char* message )
{
	dbus_error_init(&_error);
	set(name, message);
}

Error::Error( Message& m )
{
	dbus_set_error_from_message(&_error, m._message);
}

Error::~Error() throw()
{}

void Error::set( const char* name, const char* message )
{
//	_name = name;
//	_message = message;
	dbus_set_error_const(&_error, name, message);
}

const char* Error::what() const throw()
{
	return _error.message;
}

}//namespace DBus
