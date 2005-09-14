#ifndef __BTOOL_MODULE_P_H
#define __BTOOL_MODULE_P_H

#include <Python.h>
#include "../bluedebug.h"
#include "py_obj.h"
#include "btool_module.h"

namespace Bluetool
{

struct Module::Private
{
	Py::Obj		module;
	PyObject*	modclass;	// python class describing the service
	PyObject*	modservices;

	std::string name;
	std::string dbus_root;
	std::string conf_root;

	/*
	*/
	Private();
};

}//namespace Bluetool

#endif//__BTOOL_MODULE_P_H
