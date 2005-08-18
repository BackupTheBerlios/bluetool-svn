#include "bluedebug.h"
#include "btool_device.h"

namespace Bluetool
{
static char __buf[256];

static const char* __btool_gen_oname( const BdAddr& addr )
{
	snprintf(__buf,sizeof(__buf), BTOOL_DEVICES_PATH "%02X_%02X_%02X_%02X_%02X_%02X",
		addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]
	);
	return __buf;
}

const char* __btool_get_devpath( const BdAddr& addr )
{
	__btool_gen_oname(addr);
	strcat(__buf,"/");
	return __buf;
}

const char* __btool_gen_rempath( const Device* local, const BdAddr& addr )
{
	snprintf(__buf,sizeof(__buf), "%s/"BTOOL_REM_SUBDIR"%02X_%02X_%02X_%02X_%02X_%02X",
		local->oname().c_str(), addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]
	);
	return __buf;
}

Device::Device( int dev_id, const BdAddr& dev_addr ) //TODO: only the second parameter should be here
:
	HciDevice(dev_id),
	DBus::LocalObject(__btool_gen_oname(dev_addr), DBus::Connection::SystemBus()),

	_services(__btool_get_devpath(dev_addr), dev_addr)
{}

Device::~Device()
{}


Hci::RemoteDevice* Device::on_new_cache_entry( Hci::RemoteInfo& info )
{
	/* bluetooth addresses contain colons, which are not 
	   allowed in the DBUS specification
	*/
	std::string valid_addr = info.addr.to_string();
	for( uint i = 0; i < valid_addr.length(); ++i)
	{
		if(valid_addr[i] == ':')
			valid_addr[i] = '_';
	}
	std::string path = oname() + '/';
	std::string rem_name = path + BTOOL_REM_SUBDIR + valid_addr;

	RemoteDevice* hr = new RemoteDevice(this,info);

	HciDevice::DeviceInRange(*hr); //todo: move those signals in this file

	return hr;
}

RemoteDevice::RemoteDevice( Device* parent, Hci::RemoteInfo& info )
:	HciRemote(parent,info),
	SdpBrowser(parent,info.addr),
	DBus::LocalObject(__btool_gen_rempath(parent,info.addr), DBus::Connection::SystemBus())
{}

}//namespace Bluetool

