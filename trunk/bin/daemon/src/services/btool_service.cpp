#include "btool_service_p.h"
#include "btool_module_p.h"

namespace Bluetool
{

static int handle = 0;

static const char* _gen_svc_path( const std::string& dbus_root, const std::string& svc_name )
{
	static char buffer[256] = {0};

	handle = ( handle + 1 ) % 255;

	snprintf(buffer,sizeof(buffer),"%s%s%02X",dbus_root.c_str(),svc_name.c_str(),handle);

	return buffer;
}

extern PyThreadState* g_pymaintstate;

Service::Private::Private()
:		
	module( NULL ), service( NULL ), settings( NULL )
{}

Service::Private::~Private()
{
	PyEval_AcquireLock();

	PyThreadState_Swap( g_pymaintstate );

	module = NULL;
	service = NULL;
	settings = NULL;

	PyThreadState_Swap( NULL );

	PyEval_ReleaseLock();
}

Service::Service( const Module* mod, const std::string& dbus_root, const std::string& conf_root )
:
	DBus::LocalInterface( BTOOL_SVC_IFACE ),

	DBus::LocalObject( _gen_svc_path(dbus_root,mod->name()) , DBus::Connection::SystemBus() ),

	pvt( new Private )
{
	register_method( Service, GetOption );
	register_method( Service, SetOption );
	register_method( Service, Action );


	/* create service instance
	*/
	pvt->service = PyObject_CallObject( mod->pvt->modclass, NULL );

	if(!pvt->service)
	{
		PyErr_Print();
		PyEval_ReleaseLock();

		throw Dbg::Error("unable to create service instance");
	}

	pvt->settings = PyObject_GetAttrString( *pvt->service, "settings" );

	PyEval_ReleaseLock();
}

Service::~Service()
{
}

void Service::GetOption	( const DBus::CallMessage& msg )
{
	try
	{
		if(!pvt->settings)
		{
			DBus::ErrorMessage reply (msg, BTOOL_ERROR, "unsupported by plugin");
			conn().send(reply);
			return;
		}
		DBus::MessageIter ri = msg.r_iter();

		char* key = const_cast<char*>(ri.get_string());

		PyObject* value = PyDict_GetItemString( *pvt->settings, key );

		if(!value)
		{
			DBus::ErrorMessage reply (msg, BTOOL_ERROR, "no such option");
			conn().send(reply);
			return;
		}
		Py::Obj pstr ( PyObject_Str( value ) );

		const char* str = PyString_AsString( *pstr );

		DBus::ReturnMessage reply (msg);
		reply.append( 	DBUS_TYPE_STRING, &(str),
				DBUS_TYPE_INVALID
		);
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em (msg, BTOOL_ERROR, e.what());
		conn().send(em);
	}
}

void Service::SetOption	( const DBus::CallMessage& msg )
{
	try
	{
		if(!pvt->settings)
		{
			DBus::ErrorMessage reply (msg, BTOOL_ERROR, "unsupported by plugin");
			conn().send(reply);
			return;
		}
		DBus::MessageIter ri = msg.r_iter();

		char* key = const_cast<char*>(ri.get_string());
		ri++;
		char* value = const_cast<char*>(ri.get_string());

		Py::Obj pvalue ( PyString_FromString(value) );

		if( PyDict_SetItemString( *pvt->settings, key, *pvalue ) < 0 )
		{
			DBus::ErrorMessage reply (msg, BTOOL_ERROR, "can't set option");
			conn().send(reply);
			return;
		}
		DBus::ReturnMessage reply (msg);
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em (msg, BTOOL_ERROR, e.what());
		conn().send(em);
	}
}

void Service::Action	( const DBus::CallMessage& msg )
{
	/*	I can't tell how long it will take to the plugin
		to accomplish this action, so I have to do what I
		hate to do, spawn a thread

		see ActionThread::run for details
	*/

	try
	{
		ActionThread* at = new ActionThread(pvt->service,msg);

		/*	if message was valid, run!
		*/
		at->start();
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage( msg, BTOOL_ERROR, e.what() );
		conn().send(msg);
	}
}

ActionThread::ActionThread( Py::Obj& service, const DBus::CallMessage& call )
: _service(service), _call(call), _method(NULL), _args(NULL)
{
	/*	initialize thread for executing python code
	*/
	PyEval_AcquireLock();

	PyInterpreterState* mainistate = g_pymaintstate->interp;

	_thread_state = PyThreadState_New(mainistate);

	PyEval_ReleaseLock();

	/*	parse message
	*/
	DBus::MessageIter ri = call.r_iter();

	char* smethod = const_cast<char*>(ri.get_string());
	++ri;

	_method = PyObject_GetAttrString( *service, smethod );

	if(!_method || !PyCallable_Check(*_method))
	{
		PyErr_Print();
		throw Dbg::Error("action not supported");
	}

	PyObject* tuple = NULL;
	int i = 0;

	while( !ri.at_end() )
	{
		Py::Obj value( NULL );

		switch( ri.type() )
		{
			/*	you can pass only strings and integers
				to the plugin, I could make it better
				in the future, but this should cover
				the most cases
			*/
			case DBUS_TYPE_STRING:

				const char* str = ri.get_string();
				value = Py_BuildValue("s",str);
				break;

		/*	case DBUS_TYPE_UINT32:
			case DBUS_TYPE_INT32:

				int i = r.get_integer();
				value = Py_BuildValue("i",i);
				break;

		*/	default:

				Py_XDECREF(tuple);
				throw Dbg::Error("parameter type not supported");
		}

		if( !tuple )
		{
			tuple = PyTuple_New(1); //an empty tuple
		}
		else
		{
			_PyTuple_Resize( & tuple, i+1 );
		}
		if( !tuple )
		{
			PyErr_Print();
			throw Dbg::Error("Unable to allocate parameters");
		}
		PyTuple_SET_ITEM( &tuple, i, *value );
		
		++ri; ++i;
	}
	_args = tuple;
}

ActionThread::~ActionThread()
{
	/*	finalize python thread state
	*/
	PyEval_AcquireLock();

	PyThreadState_Swap(NULL);
	PyThreadState_Clear(_thread_state);
	PyThreadState_Delete(_thread_state);

	PyEval_ReleaseLock();
}

void ActionThread::run()
{
	PyEval_AcquireLock();

	PyThreadState_Swap(_thread_state);

	/*	jump into python code
	*/
	Py::Obj result ( PyObject_CallObject( *_method, *_args ) );

	if(!result)
	{
		PyErr_Print();
	}

	PyThreadState_Swap(NULL);

	PyEval_ReleaseLock();

	/*	parse return value and respond to caller
	*/
	DBus::ReturnMessage reply(_call);

	DBus::Connection::SystemBus().send(reply);
}

void ActionThread::on_end()
{
	delete this;	//DON'T allocate this class on stack!
}


}//namespace Bluetool
