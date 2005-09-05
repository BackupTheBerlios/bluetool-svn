#include "bluedebug.h"
#include "btool_device.h"

namespace Bluetool
{
static char __buf[256];

/*	TODO: write some DECENT functions to generate path names 
	for dbus objects
*/

static const char* _gen_oname( const BdAddr& addr )
{
	snprintf(__buf,sizeof(__buf), BTOOL_DEVICES_PATH "%02X_%02X_%02X_%02X_%02X_%02X",
		addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]
	);
	return __buf;
}

static const char* _get_devpath( const BdAddr& addr )
{
	_gen_oname(addr);
	strcat(__buf,"/");
	return __buf;
}

static const char* _gen_rem_name( const Device* local, const BdAddr& addr )
{
	snprintf(__buf,sizeof(__buf), "%s/"BTOOL_REM_SUBDIR"%02X_%02X_%02X_%02X_%02X_%02X",
		local->oname().c_str(), addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]
	);
	return __buf;
}

static const char* _gen_conn_name( const RemoteDevice* endpoint, u16 handle )
{
	snprintf(__buf,sizeof(__buf), "%s/"BTOOL_CONN_SUBDIR"0x%04X",
		endpoint->oname().c_str(), handle
	);
	return __buf;
}

Device::Device( int dev_id )
:
	HciDevice(dev_id),
	DBus::LocalObject(_gen_oname(HciDevice::addr()), DBus::Connection::SystemBus())
{
	_services = new ServiceDatabase( this->oname(), std::string(""));
}

Device::~Device()
{
	delete _services;
}

Hci::RemoteDevice* Device::on_new_cache_entry( Hci::RemoteInfo& info )
{
	RemoteDevice* hr = new RemoteDevice(this,info);

	//this->DeviceInRange(*hr); //todo: move those signals in this file

	return hr;
}

RemoteDevice::RemoteDevice( Device* parent, Hci::RemoteInfo& info )
:	HciRemote(parent,info),
	SdpBrowser(parent->addr(),info.addr),
	DBus::LocalObject(_gen_rem_name(parent,info.addr), DBus::Connection::SystemBus()),

	_parent(parent)
{
	_services = new ServiceDatabase( this->oname(), std::string(""));

	DBus::SignalMessage sig("DeviceInRange");

	const char* name = oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);
	_parent->emit_signal(sig);
}

RemoteDevice::~RemoteDevice()
{
	DBus::SignalMessage sig("DeviceOutOfRange");

	const char* name = oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);
	_parent->emit_signal(sig);


	delete _services;
}

Hci::Connection* RemoteDevice::on_new_connection( Hci::ConnInfo& info )
{
	Connection* c = new Connection(this,info);

	return c;
}

Connection::Connection( RemoteDevice* endpoint, Hci::ConnInfo& info )
:	HciConnection(endpoint, info),
	DBus::LocalObject(_gen_conn_name(endpoint,info.handle), DBus::Connection::SystemBus())
{}

}//namespace Bluetool

