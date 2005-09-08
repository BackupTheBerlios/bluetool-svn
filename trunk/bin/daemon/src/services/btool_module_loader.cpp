#include <Python.h>

#include "btool_module_loader.h"
#include "../bluedebug.h"

namespace Bluetool
{

PyThreadState* g_pymaintstate;

void ModuleLoader::init()
{
	Py_InitializeEx(0);
	PyEval_InitThreads();	//note, this implicitly acquires the lock!

	g_pymaintstate = PyThreadState_Get();

	PyEval_ReleaseLock();
}

void ModuleLoader::finalize()
{
	PyEval_AcquireLock();

	PyThreadState_Swap( g_pymaintstate );

	Py_Finalize();
}

Module* ModuleLoader::load_module
(
	const std::string& name,
	const std::string& dbus_root,
	const std::string& conf_root
)
{
	return new Module(name,dbus_root,conf_root);
}

Instance* ModuleLoader::instantiate
(
	const Module* mod,
	const std::string& dbus_root,
	const std::string& conf_root
)
{
	return new Instance(mod,dbus_root,conf_root);
}

}//namespace Bluetool
