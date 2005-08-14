#include "bluedebug.h"
#include "btool_device.h"

namespace Bluetool
{
static char __buf[256];

static const char* __btool_gen_oname( const BdAddr& addr )
{
	snprintf(__buf,sizeof(__buf), BTOOL_DEVICES_PATH "%X_%X_%X_%X_%X_%X",
		addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]
	);
	return __buf;
}

static const char* __btool_gen_oroot( const BdAddr& addr )
{
	__btool_gen_oname(addr);
	strcat(__buf,"/");
	return __buf;
}

Device::Device( int dev_id, const BdAddr& dev_addr ) //TODO: only the second parameter should be here
:
	HciDevice(dev_id),
	DBus::LocalObject(__btool_gen_oname(dev_addr), DBus::Connection::SystemBus()),

	_services(__btool_gen_oroot(dev_addr), dev_addr)
{}

}//namespace Bluetool

