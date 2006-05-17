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


#ifndef __CBUS_OBJECT_H
#define __CBUS_OBJECT_H

#include <string>
#include <list>

namespace DBus
{
 	class Object;
	typedef std::list<Object*>	ObjectPList;

	class LocalObject;

	class RemoteObject;
}

#include "cbusfilter.h"
#include "cbusconnection.h"
#include "cbusmessage.h"
#include "cbusinterface.h"

namespace DBus
{

class Object : protected virtual IfaceTracker
{
protected:

	Object( const char* pathname, Connection& conn );
	
public:

	virtual ~Object();

	inline const std::string& oname() const;

	const Object* object() const;
	
 	inline Connection& conn();

private:

	virtual bool handle_message( const Message& ) = 0;
	virtual void register_obj() = 0;
	virtual void unregister_obj() = 0;

private:

	Connection	_conn;
	std::string	_name;

//friend class Connection;
};

/*
*/

Connection& Object::conn()
{
	return _conn;
}


const std::string& Object::oname() const
{
	return _name;
}

/*
*/

class LocalObject : public Object
{
public:

	LocalObject( const char* name, Connection& conn );

	~LocalObject();

	void remit_signal( SignalMessage& );

private:

	LocalObject( const LocalObject& );

	bool handle_message( const Message& );

	void register_obj();
	void unregister_obj();

private:

	static DBusObjectPathVTable _vtable;

	static void unregister_function_stub( DBusConnection*, void* );
	static DBusHandlerResult message_function_stub( DBusConnection*, DBusMessage*, void* );
};

/*
*/

class RemoteObject : public Object
{
public:

	RemoteObject( const char* name, Connection& conn );

	~RemoteObject();

private:

	RemoteObject( const RemoteObject& );

	bool handle_message( const Message& );

	void register_obj();
	void unregister_obj();

private:
	
	Filter	_receiver;

//friend class Connection;
};

}//namespace DBus

#endif//__CBUS_OBJECT_H
