#include "service_database.h"

#define BLUETOOL_CONF_PATH "/tmp/bluetool_cache/"

namespace Bluetool
{

static const char* __gen_svcdb_oname( const char* parent )
{
	static char buf[256];
	snprintf(buf,sizeof(buf),"%s/servicedb",parent);
	return buf;
}

ServiceDatabase::ServiceDatabase( const char* dbus_root, const BdAddr& dev_addr )
:	DBus::LocalInterface( BTOOL_SERVICEDB_IFACE ),
	DBus::LocalObject( __gen_svcdb_oname("/servicedb"), DBus::Connection::SystemBus() )
{
	std::string uscore_addr = dev_addr.to_string();
	for( uint i = 0; i < uscore_addr.length(); ++i )
	{
		if(uscore_addr[i] == ':')
			uscore_addr[i] = '_';
	}

	_conf_root = BLUETOOL_CONF_PATH + uscore_addr +"/";

	_pan_service = new PanService(dbus_root,_conf_root,dev_addr);
}

ServiceDatabase::~ServiceDatabase()
{
	delete _pan_service;
}

}//namespace Bluetool
