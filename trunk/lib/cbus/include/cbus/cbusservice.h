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


#ifndef __CBUS_SERVICE_H
#define __CBUS_SERVICE_H

/*	TODO: remove this file
*/

#include <string>

namespace DBus
{
class Service;
}

#include "cbusconnection.h"
#include "cbusobject.h"

namespace DBus
{

class RemoteService
{
public:

	RemoteService( Connection& c, const char* name );
	
	virtual ~RemoteService();
	
	static RemoteService get( Connection& c, const char* name );

	inline const std::string& name();

//	RemoteObject& get_object();

private:

	Connection		_conn;
	const std::string	_name;
};

/*
*/

const std::string& RemoteService::name()
{
	return _name;
}

}//namespace DBus

#endif//__CBUS_SERVICE_H
