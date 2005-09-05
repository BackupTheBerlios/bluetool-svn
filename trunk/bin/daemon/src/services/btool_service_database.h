#ifndef __BTOOL_SERVICE_DATABASE_H
#define __BTOOL_SERVICE_DATABASE_H

#include <cbus/cbus.h>
#include "btool_service.h"
#include "btool_service_loader.h"
#include "../btool_names.h"

namespace Bluetool
{
	class ServiceDatabase;
}

#include "../btool_device.h"

namespace Bluetool
{

class ServiceDatabase : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	ServiceDatabase( const std::string& parent, const std::string& conf_root );

	~ServiceDatabase();

	/*	methods
	*/
	void ListServices	( const DBus::CallMessage& );

	void LoadService	( const DBus::CallMessage& );

	void UnloadService	( const DBus::CallMessage& );

private:

	ServicePList	_services;	// 'service instances' should be a more appropriate name
	std::string	_conf_root;
};

}//namespace Bluetool

#endif//__BTOOL_SERVICE_DATABASE_H
