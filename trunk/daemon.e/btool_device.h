#ifndef __BTOOL_DEVICE_H
#define __BTOOL_DEVICE_H

#include <map>

#include "../libbluetool/bdaddr.h"
#include "../cbus/cbus.h"
#include "hci/hci_device.h"
#include "services/database/service_database.h"
#include "btool_names.h"

namespace Bluetool
{

class Device;
typedef std::map<int, Device*>	DevicePTable;

class Device : public HciDevice, public DBus::LocalObject
{
public:
	Device( int dev_id, const BdAddr& );

private:
	ServiceDatabase	_services;
};

}//namespace Bluetool

#endif//__BTOOL_DEVICE_H
