#include "btool_service_database.h"

#define BLUETOOL_CONF_PATH "/tmp/bluetool_cache/"

namespace Bluetool
{

static const char* __gen_svcdb_oname( const char* parent )
{
	static char buf[256];
	snprintf(buf,sizeof(buf),"%sservicedb",parent);
	return buf;
}

ServiceDatabase::ServiceDatabase( const char* dbus_root, const BdAddr& dev_addr )
:	DBus::LocalInterface( BTOOL_SERVICEDB_IFACE ),
	DBus::LocalObject( __gen_svcdb_oname(dbus_root), DBus::Connection::SystemBus() )
{
	std::string uscore_addr = dev_addr.to_string();
	for( uint i = 0; i < uscore_addr.length(); ++i )
	{
		if(uscore_addr[i] == ':')
			uscore_addr[i] = '_';
	}

	_conf_root = BLUETOOL_CONF_PATH + uscore_addr +"/";

//	Service* new_service = ServiceLoader::load_service("demoservice",dbus_root,_conf_root);

//	if(new_service) _services.push_back(new_service);
}

ServiceDatabase::~ServiceDatabase()
{
	ServicePList::iterator si = _services.begin();
	while( si != _services.end() )
	{
		delete *si;
		++si;
	}
}

}//namespace Bluetool
