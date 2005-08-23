#include <Python.h>

#include "btool_service_loader.h"

namespace Bluetool
{

void ServiceLoader::init()
{
	Py_InitializeEx(0);
}

void ServiceLoader::finalize()
{
	Py_Finalize();
}

Service* ServiceLoader::load_service
(
	const std::string& name,
	const std::string& dbus_root,
	const std::string& conf_root
)
{
	Service* svc;
	try
	{
		svc = new Service(name,dbus_root,conf_root);
	}
	catch(...)
	{	
		delete svc;
		svc = NULL;
	}
	return svc;
}

}//namespace Bluetool
