#ifndef __BTOOL_ROOT_SERVICE_H
#define __BTOOL_ROOT_SERVICE_H

#include <cbus/cbus.h>
#include "btool_device_manager.h"
#include "btool_names.h"

namespace Bluetool
{

class RootService
{
public:
	RootService();

private:
	DBus::Connection _bus_connection;
};

}//namespace Bluetool

#endif//__BTOOL_ROOT_SERVICE_H
