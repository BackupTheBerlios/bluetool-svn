#include "btool_module_p.h"

namespace Bluetool
{

Module::Private::Private()
: module( NULL )
{}

static const char* _gen_module_path( const std::string& dbus_root, const std::string& mod_name )
{
	static char buffer[256] = {0};

	snprintf(buffer,sizeof(buffer),"%s%s",dbus_root.c_str(),mod_name.c_str());

	return buffer;
}

//extern PyThreadState* g_pymaintstate;

Module::Module( const std::string& name, const std::string& dbus_root, const std::string& conf_root )
:
	DBus::LocalInterface( BTOOL_MOD_IFACE ),

	DBus::LocalObject( _gen_module_path(dbus_root,name) , DBus::Connection::SystemBus() ),

	pvt( new Private )
{
//	register_method( Module, Instance );
	register_method( Module, Description );

	PyEval_AcquireLock();

	/* add the service path to python's search path
	*/	
	PyRun_SimpleString
	(
		"import sys\n"
		"import os\n"
		"sys.path.append(os.getcwd()+'/src/services')\n" 
		// XXX: put it in a place to define at configure-time
		//"print sys.path\n"
	);

	/* load module into embedded interpreter
	*/
	Py::Obj pname ( PyString_FromString( name.c_str() ) );
	pvt->module = PyImport_Import( *pname );

	if(!pvt->module)
	{
		PyErr_Print();
		PyEval_ReleaseLock();

		throw Dbg::Error("unable to load module");
	}
	/* get a dictionary to browse module contents
	*/
	PyObject* dict = PyModule_GetDict( *pvt->module );

	std::string service = "bluetool_";
	service += name;

	pvt->modclass = PyDict_GetItemString( dict,service.c_str() );

	if(!pvt->modclass)
	{
		PyErr_Print();
		PyEval_ReleaseLock();

		throw Dbg::Error("unable to find service class");
	}
	PyEval_ReleaseLock();
}

Module::~Module()
{}

const std::string& Module::name() const
{
	return pvt->name;
}
/*
void Module::Instance ( const DBus::CallMessage& )
{
}
*/
void Module::Description ( const DBus::CallMessage& msg )
{
	try
	{
		Py::Obj desc ( PyObject_GetAttrString(pvt->modclass, "__doc__") );
		if(!desc)
			throw Dbg::Error("Not found");

		const char* sdesc = PyString_AsString(*desc);

		if(!sdesc)
			sdesc = "";

		DBus::ReturnMessage reply (msg);
		reply.append( DBUS_TYPE_STRING, &(sdesc), DBUS_TYPE_INVALID );

		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em (msg, BTOOL_ERROR, e.what());
		conn().send(em);
	}
}

}//namespace Bluetool
