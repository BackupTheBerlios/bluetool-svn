#ifndef __CBUS_SERVICE_H
#define __CBUS_SERVICE_H

/*	UPDATE: maybe we don't need this class
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
