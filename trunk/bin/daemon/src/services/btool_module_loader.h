#ifndef __BTOOL_PYTHON_LOADER_H
#define __BTOOL_PYTHON_LOADER_H

#include "btool_instance.h"
#include "btool_module.h"

namespace Bluetool
{

class ModuleLoader
{
public:
	static void init();
	static void finalize();

	static Module* load_module
	(
		const std::string& name,
		const std::string& dbus_root,
		const std::string& conf_root
	);

	static Instance* instantiate
	(
		const Module* mod,
		const std::string& dbus_root,
		const std::string& conf_root
	);
};

}//namespace Bluetool

#endif//__BTOOL_PYTHON_LOADER_H
