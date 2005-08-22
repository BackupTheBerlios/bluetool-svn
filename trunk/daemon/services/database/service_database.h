#ifndef __BTOOL_SERVICEDB_H
#define __BTOOL_SERVICEDB_H

#include "../btool_service_loader.h"
#include "../../btool_names.h"
#include "../../../cbus/cbus.h"
#include "../../../libbluetool/bdaddr.h"

namespace Bluetool
{

class ServiceDatabase : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	ServiceDatabase( const char* dbus_root, const BdAddr& dev_addr );
	~ServiceDatabase();

	void ListServices	( const DBus::CallMessage& );

private:
	std::string _conf_root;

};

}//namespace Bluetool

#endif//__BTOOL_SERVICEDB_H
