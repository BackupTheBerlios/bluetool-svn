#include <Python.h>
#include "btool_service.h"

namespace Bluetool
{

struct Service::Private
{
	bool started;

	PyObject* module;
	PyObject* service;
};

Service::Service
(
	const std::string& name,
	const std::string& dbus_root,
	const std::string& conf_root
)
:	/* load interface (common to all services)
	*/
	DBus::LocalInterface( BTOOL_SVC_IFACE ), //TODO: create a service-specific interface too

	/* create service object
	*/
	DBus::LocalObject
	(
		(dbus_root + BTOOL_SVC_SUBDIR + name).c_str(),
		DBus::Connection::SystemBus()
	),

	/* load configuration file
	*/
	settings( (conf_root + name + ".conf").c_str() )
{
	pvt = new Private();
	pvt->started=false;
	pvt->module=NULL;
	pvt->service=NULL;

	/* register standard methods
	*/
	register_method( Service, Start );
	register_method( Service, GetOption );
	register_method( Service, SetOption );
	register_method( Service, Stop );

	/* add the service path to python's search path
	*/	
	PyRun_SimpleString
	(
		"import sys\n"
		"import os\n"
		"sys.path.append(os.getcwd()+'/services')\n"
		//"print sys.path\n"
	);

	/* load module into embedded interpreter
	*/
	PyObject* pname = PyString_FromString(name.c_str());
	pvt->module = PyImport_Import(pname);
	Py_DECREF(pname);

	if(!pvt->module)
	{
		PyErr_Print();
		throw "unable to load module";
	}

	/* get a dictionary to browse module contents
	*/
	PyObject* dict = PyModule_GetDict(pvt->module);
	std::string service = "bluetool_"+name;
	PyObject* svc_item = PyDict_GetItemString(dict,service.c_str());

	if(!svc_item)
	{
		Py_DECREF(pvt->module);

		PyErr_Print();
		throw "unable to load module";
	}

	/* create service instance
	*/
	pvt->service = PyObject_CallObject(svc_item, NULL);

	if(!pvt->service)
	{
		Py_DECREF(pvt->module);

		PyErr_Print();
		throw "unable to create service instance";
	}
}

Service::~Service()
{
	delete pvt;
//	conn().disconnect();
}


bool Service::started()
{
	return pvt->started;
}

void Service::GetOption( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* key = ri.get_string();

		const char* value = settings.get_option(key).c_str();

		DBus::ReturnMessage reply(msg);

		reply.append( DBUS_TYPE_STRING, &value,
			      DBUS_TYPE_INVALID
		);
		conn().send(reply);
	}
	catch(...)
	{}
}

void Service::SetOption( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* key = ri.get_string();

		const char* value = ri.get_string();

		bool res = settings.set_option(key,value);

		DBus::ReturnMessage reply(msg);

		reply.append( DBUS_TYPE_BOOLEAN, &res,
			      DBUS_TYPE_INVALID
		);
		conn().send(reply);
	}
	catch(...)
	{}
}

void Service::Start( const DBus::CallMessage& msg )
{
	bool res;

	if(pvt->started)
	{
		res = true;
	}
	else
	{
		PyObject* ret = PyObject_CallMethod(pvt->service, "Start", NULL);

		if(!PyBool_Check(ret))
		{
			throw "Start() returned non boolean";
		}
		else
		{
			if(ret == Py_True)
			{
				pvt->started = true;
				this->ServiceStarted();
			}
		}
		Py_DECREF(ret);
	}
	DBus::ReturnMessage reply(msg);

	reply.append( DBUS_TYPE_BOOLEAN, &res,
		      DBUS_TYPE_INVALID
	);
	conn().send(reply);
}

void Service::Stop( const DBus::CallMessage& msg )
{
	bool res;

	if(!pvt->started)
	{
		res = true;
	}
	else
	{
		PyObject* ret = PyObject_CallMethod(pvt->service, "Stop", NULL);

		if(!PyBool_Check(ret))
		{
			throw "Stop() returned non boolean";
		}
		else
		{
			if(ret == Py_True)
			{
				pvt->started = false;
				this->ServiceStopped();
			}
		}
		Py_DECREF(ret);
	}
	DBus::ReturnMessage reply(msg);

	reply.append( DBUS_TYPE_BOOLEAN, &res,
		      DBUS_TYPE_INVALID
	);
	conn().send(reply);
}

void Service::ServiceStarted()
{
	const char* sname = oname().c_str();

	DBus::SignalMessage msg (sname, iname().c_str(), "ServiceStarted");

	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}

void Service::ServiceStopped()
{
	const char* sname = oname().c_str();

	DBus::SignalMessage msg (sname, iname().c_str(), "ServiceStopped");

	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}

}//namespace Bluetool
