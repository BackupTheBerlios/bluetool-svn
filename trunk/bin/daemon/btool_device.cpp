#include "bluedebug.h"
#include "btool_device.h"

namespace Bluetool
{
static char __buf[256];

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

Device::Device( int dev_id, const BdAddr& dev_addr ) //TODO: only the second parameter should be here
:
	HciDevice(dev_id),
	DBus::LocalObject(_gen_oname(dev_addr), DBus::Connection::SystemBus()),

	_services(_get_devpath(dev_addr), dev_addr)
{}

Device::~Device()
{}

/*	signals
*/

void Device::DeviceInRange( const RemoteDevice& hr )
{
	/* todo: I don't like this way of sending signals
	*/
	DBus::SignalMessage sig
	(
		BTOOL_DEVMAN_PATH, //TODO
		iname().c_str(),
		"DeviceInRange"
	);

	const char* name = hr.oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);

	conn().send(sig);
}

void Device::DeviceOutOfRange( const RemoteDevice& hr )
{
	/* todo: I don't like this way of sending signals
	*/
	DBus::SignalMessage sig
	(
		BTOOL_DEVMAN_PATH, //TODO
		iname().c_str(),
		"DeviceOutOfRange"
	);

	const char* name = hr.oname().c_str();

	sig.append
	(
		DBUS_TYPE_STRING, &(name),
		DBUS_TYPE_INVALID
	);

	conn().send(sig);
}

Hci::RemoteDevice* Device::on_new_cache_entry( Hci::RemoteInfo& info )
{
	RemoteDevice* hr = new RemoteDevice(this,info);

	this->DeviceInRange(*hr); //todo: move those signals in this file

	return hr;
}

RemoteDevice::RemoteDevice( Device* parent, Hci::RemoteInfo& info )
:	HciRemote(parent,info),
	SdpBrowser(parent,info.addr),
	DBus::LocalObject(_gen_rem_name(parent,info.addr), DBus::Connection::SystemBus()),

	_parent(parent)
{}

RemoteDevice::~RemoteDevice()
{
	_parent->DeviceOutOfRange(*this);
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

