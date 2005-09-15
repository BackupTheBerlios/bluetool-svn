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
#include <cbus/cbusmessage.h>
#include <cbus/cbuserror.h>

namespace DBus
{

Error::Error()
: Dbg::Error( NULL )
{
	dbus_error_init(&_error);
}

Error::Error( DBusError* error )
: Dbg::Error( NULL )
{
	dbus_move_error(error, &_error);

#ifdef _DEBUG
	if( is_set() ) cbus_dbg("error %s: %s",name(), message() );
#endif
}

Error::Error( const char* name, const char* message )
: Dbg::Error( NULL )
{
	dbus_error_init(&_error);
	set(name, message);
}

Error::Error( Message& m )
: Dbg::Error( NULL )
{
	dbus_set_error_from_message(&_error, m._message);
}

//Error::~Error() throw()
//{}

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
