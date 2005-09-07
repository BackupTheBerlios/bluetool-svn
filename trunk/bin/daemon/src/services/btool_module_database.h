#ifndef __BTOOL_MODULE_DATABASE_H
#define __BTOOL_MODULE_DATABASE_H

#include <cbus/cbus.h>
#include "btool_module.h"
#include "btool_service.h"
#include "btool_service_loader.h"
#include "../btool_names.h"

namespace Bluetool
{
	class ModuleDatabase;
}

#include "../btool_device.h"

namespace Bluetool
{

class ModuleDatabase : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	ModuleDatabase( const std::string& parent, const std::string& conf_root );

	~ModuleDatabase();

	/*	methods
	*/
	void ListModules ( const DBus::CallMessage& );

//	void LoadModule	 ( const DBus::CallMessage& );

//	void UnloadModule	( const DBus::CallMessage& );

private:

	ModulePList	_modules;
	std::string	_conf_root;
};

}//namespace Bluetool

#endif//__BTOOL_MODULE_DATABASE_H
