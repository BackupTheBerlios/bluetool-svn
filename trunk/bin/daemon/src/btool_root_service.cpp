#include "btool_root_service.h"

namespace Bluetool
{

RootService::RootService()
:	_bus_connection(DBus::Connection::SystemBus())
{
	_bus_connection.request_name(BTOOL_ROOT_SERVICE);
}

}//namespace Bluetool
