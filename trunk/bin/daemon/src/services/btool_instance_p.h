#ifndef __BTOOL_INSTANCE_P_H
#define __BTOOL_INSTANCE_P_H

#include <Python.h>
#include <common/thread.h>
#include <list>
#include "btool_instance.h"
#include "py_obj.h"
#include "../bluedebug.h"

namespace Bluetool
{

class ActionThread;

typedef std::list<ActionThread*> ActionThreadPList;

class ActionThread : public Thread
{
public:

	ActionThread( Py::Obj& service, const DBus::CallMessage& call );
	~ActionThread();

	void run();

	void on_end();

private:

	Py::Obj _service;
	Py::Obj _method;
	Py::Obj _args;

	PyThreadState* _thread_state;

	DBus::CallMessage _call;
};

struct Instance::Private
{
	/* members
	*/
	Py::Obj	module;
	Py::Obj	service;
	Py::Obj settings;

	/* [c|d]tors
	*/
	Private();
	~Private();
};

}//namespace Bluetool

#endif//__BTOOL_INSTANCE_P_H
