#ifndef __BTOOL_PYTHON_LOADER_H
#define __BTOOL_PYTHON_LOADER_H

#include "btool_service.h"

namespace Bluetool
{

class ServiceLoader
{
public:
	static void init();
	static void finalize();

	static Service* load_service
	(
		const std::string& name,
		const std::string& dbus_root,
		const std::string& conf_root
	);
};

}//namespace Bluetool

#endif//__BTOOL_PYTHON_LOADER_H
