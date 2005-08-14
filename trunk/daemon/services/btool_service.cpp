#include "btool_service.h"

BluetoolService::BluetoolService
(
	const std::string& dbus_root,
	const std::string& conf_root,
	const std::string& name
)
:	DBus::LocalInterface( BTOOL_SVC_IFACE ),
	DBus::LocalObject
	(
		(dbus_root + BTOOL_SVC_SUBDIR + name).c_str(),
		DBus::Connection::SystemBus()
	),
	settings( (conf_root + name + ".conf").c_str() ),

	_started(false)
{
	register_method( BluetoolService, GetOption );
	register_method( BluetoolService, SetOption );
	register_method( BluetoolService, Stop );
}

BluetoolService::~BluetoolService()
{
//	conn().disconnect();
}

void BluetoolService::GetOption( const DBus::CallMessage& msg )
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

void BluetoolService::SetOption( const DBus::CallMessage& msg )
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

void BluetoolService::Start( const DBus::CallMessage& msg )
{
	bool res;

	if(_started)
	{
		res = true;
	}
	else
	{
		res = this->start_service();
		if(res)
		{
			_started = true;
			this->ServiceStarted();
		}
	}
	DBus::ReturnMessage reply(msg);

	reply.append( DBUS_TYPE_BOOLEAN, &res,
		      DBUS_TYPE_INVALID
	);
	conn().send(reply);
}

void BluetoolService::Stop( const DBus::CallMessage& msg )
{
	bool res;

	if(!_started)
	{
		res = true;
	}
	else
	{
		res = this->stop_service();
		if(res)
		{
			_started = false;
			this->ServiceStopped();
		}
	}
	DBus::ReturnMessage reply(msg);

	reply.append( DBUS_TYPE_BOOLEAN, &res,
		      DBUS_TYPE_INVALID
	);
	conn().send(reply);
}

void BluetoolService::ServiceStarted()
{
	const char* sname = oname().c_str();

	DBus::SignalMessage msg (sname, iname().c_str(), "ServiceStarted");

	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}

void BluetoolService::ServiceStopped()
{
	const char* sname = oname().c_str();

	DBus::SignalMessage msg (sname, iname().c_str(), "ServiceStopped");

	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}
