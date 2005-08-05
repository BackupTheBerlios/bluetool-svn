#include "haldebug.h"
#include "haldevice.h"

HalDevice::HalDevice( const char* udi )
:	
	/*	bind to remote object
	*/
	DBus::RemoteInterface( HALDEV_IFACE ),
	DBus::RemoteObject( udi, DBus::Connection::SystemBus() ),

	/*	private stuff
	*/
	_udi(udi)
{
	
}

bool HalDevice::QueryCapability( const char* capability )
{
	/*	build request
	*/	
	DBus::CallMessage msg
	(
		HAL_PATH,
		_udi.c_str(),
		HALDEV_IFACE,
		"QueryCapability"
	);

//	DBus::MessageIter iter = msg.w_iter();
//	iter.append_string(capability);
 	msg.write("%s", DBUS_TYPE_STRING, &capability, DBUS_TYPE_INVALID);

	/*	invoke remote method
	*/
	try
	
	{ 	
		DBus::Message reply = conn().send_blocking(msg, 2000);

	
		DBus::MessageIter ri = reply.r_iter();
		return ri.get_bool();
 	}
 	catch( std::exception& e )
 	{
	
 		hal_dbg("%s",e.what());
	
 		return false;
	}	
}

bool HalDevice::PropertyExists( const char* property )
{
	/*	build request
	*/	
	DBus::CallMessage msg
	(
		HAL_PATH,
		_udi.c_str(),
		HALDEV_IFACE,
		"PropertyExists"
	);

	DBus::MessageIter iter = msg.w_iter();
	iter.append_string(property);

	/*	invoke remote method
	*/
	DBus::Message reply = conn().send_blocking(msg, 1000);

	//hal_dbg("returned");
	/*	wait...
	*/
	//if(reply.is_error())
	//	throw DBus::Error(reply);

	DBus::MessageIter ri = reply.r_iter();
	return ri.get_bool();
}

const char* HalDevice::GetPropertyString( const char* property )
{
	/*	build request
	*/	
	DBus::CallMessage msg
	(
		HAL_PATH,
		_udi.c_str(),
		HALDEV_IFACE,
		"GetPropertyString"
	);

//	DBus::MessageIter iter = msg.w_iter();
//	iter.append_string(property);
 	msg.write("%s", DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

	/*	invoke remote method
	*/
	DBus::Message reply = conn().send_blocking(msg, 1000);

	//hal_dbg("returned");
	/*	wait...
	*/
	//if(reply.is_error())
	//	throw DBus::Error(reply);

	DBus::MessageIter ri = reply.r_iter();
	return ri.get_string();
}
