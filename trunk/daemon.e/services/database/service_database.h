#ifndef __BTOOL_SERVICEDB_H
#define __BTOOL_SERVICEDB_H

#include "../../../cbus/cbus.h"
#include "../pan/pan_service.h"

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

	/* list of services
	*/
	PanService* _pan_service;
};

}//namespace Bluetool

#endif//__BTOOL_SERVICEDB_H
