#include "btool_service.h"

BluetoolService::BluetoolService( const std::string name )
:	DBus::LocalInterface( BTOOL_SERVICE_IFACE ),
	DBus::LocalObject( (BTOOL_SERVICE_SUBPATH + name).c_str(), DBus::Connection::ActivationBus() )
{
	conn().request_name( (BTOOL_SERVICE_ROOT + name).c_str() );

	register_method( BluetoolService, Stop );

	this->ServiceStarted();
}

BluetoolService::~BluetoolService()
{
	conn().disconnect();
}

void BluetoolService::Stop( const DBus::CallMessage& )
{
	this->ServiceStopped();
	conn().disconnect();
}

void BluetoolService::ServiceStarted()
{
	DBus::SignalMessage msg (oname().c_str(), iname().c_str(), "ServiceStarted");

	const char* sname = oname().c_str();
	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}

void BluetoolService::ServiceStopped()
{
	DBus::SignalMessage msg (oname().c_str(), iname().c_str(), "ServiceStopped");

	const char* sname = oname().c_str();
	msg.append( DBUS_TYPE_STRING, &sname,
		    DBUS_TYPE_INVALID
	);
	conn().send(msg);
}
