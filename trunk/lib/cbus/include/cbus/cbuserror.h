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


#ifndef __CBUS_ERROR_H
#define __CBUS_ERROR_H

//#include <string>
#include <eeple/error.h>
#include <dbus/dbus.h>

namespace DBus
{
class Error;
class Message;
}

namespace DBus
{

class Error : public Dbg::Error
{
public:
	Error();

	Error( const char* name, const char* message );
	
	explicit Error( DBusError* error );

	Error( Message& );

	//~Error() throw();

	const char* what() const throw();

	const char* name()
	{
		return _error.name;
	}

	const char* message()
	{
		return _error.message;
	}

	operator bool()
	{
		return is_set();
	}

	operator DBusError*() //ouch!
	{
		return &_error;
	}

	bool is_set()
	{
		return dbus_error_is_set(&_error);
	}

	void set( const char* name, const char* message );	//todo: parameters MUST be statics

//	void set( const char* name, const char* format, ... )
//	{
//		return dbus_set_error(&_error, name, message);
//	}

private:
	DBusError	_error;
//	std::string	_name;
//	std::string	_message;
};

}//namespace DBus

#endif//__CBUS_ERROR_H
