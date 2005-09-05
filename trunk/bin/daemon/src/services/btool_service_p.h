#ifndef __BTOOL_SERVICE_P_H
#define __BTOOL_SERVICE_P_H

#include <Python.h>
#include <common/thread.h>
#include <list>
#include "btool_service.h"
#include "../bluedebug.h"

/*	simple class to automate the chore
	of reference counting on python objects
	( XXX: move it in a header on its own )
*/

namespace Py
{

class Obj
{
public:
	explicit Obj( PyObject* o ) : _o ( o )//, rc(1)
	{
		//blue_dbg("%p=)%d",_o,rc);
	}

	Obj( const Obj& o )
	{
		_o = o._o;
		//rc = o.rc;
		ref();
	}

	~Obj()
	{
		unref();
	}

	Obj& operator = ( PyObject* o )
	{
		unref();
		//rc = 1;
		_o = o;
		return *this;
	}

	Obj& operator = ( const Obj& o )
	{
		if( this != &o )
		{
			unref();
			_o = o._o;
			//rc = o.rc;
			ref();
		}
		return *this;
	}

	PyObject* operator * ()
	{
		return _o;
	}

	operator bool()
	{
		return _o != NULL;
	}

private:
	void ref()
	{
		//++rc;
		//blue_dbg("%p+)%d",_o,rc);
		Py_XINCREF( _o );
	}

	void unref()
	{
		//--rc;
		//blue_dbg("%p-)%d",_o,rc);
		Py_XDECREF( _o );
	}

private:
	PyObject* _o;
	//int rc;
};

}

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

struct Service::Private
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

#endif//__BTOOL_SERVICE_P_H
