#ifndef __BTOOL_MODULE_DATABASE_H
#define __BTOOL_MODULE_DATABASE_H

#include <sys/types.h>
#include <dirent.h>
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

//	void UnloadModule( const DBus::CallMessage& );

	inline const ModulePList& modules() const;

	Module* find_module_from_path( const char* obj_path );

	Module* find_module_from_serviceid( u16 id );

private:

	ModulePList	_modules;
	ServicePList	_instances;
	std::string	_conf_root;
};

const ModulePList& ModuleDatabase::modules() const
{
	return _modules;
}

}//namespace Bluetool

#endif//__BTOOL_MODULE_DATABASE_H
