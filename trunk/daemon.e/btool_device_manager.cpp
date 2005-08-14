#include "bluedebug.h"
#include "btool_device_manager.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <sys/ioctl.h>

namespace Bluetool
{

static BdAddr __get_dev_addr( int dev_id )
{
	hci_dev_info di;
	memset(&di, 0, sizeof(di));
	di.dev_id = dev_id;

	Hci::Socket ctl;
	if(ioctl(ctl.handle(), HCIGETDEVINFO, (long)&di) < 0)
		blue_dbg("unable to get address of device");

	return BdAddr(di.bdaddr.b);
}

DeviceManager::DeviceManager()
:		
	DBus::LocalInterface ( BTOOL_DEVMAN_IFACE ),
	DBus::LocalObject    ( BTOOL_DEVMAN_PATH, DBus::Connection::SystemBus() )
{
	/*	export all methods in the interface
	*/
	register_method( DeviceManager, ListDevices );
	register_method( DeviceManager, EnableDevice );
	register_method( DeviceManager, DisableDevice );
	register_method( DeviceManager, ResetDevice );

	/*
	*/
	if(!_evt_socket.bind(HCI_DEV_NONE))
		throw Hci::Exception();

	Hci::Filter f;
	f.set_event(EVT_STACK_INTERNAL);
	f.set_type(HCI_EVENT_PKT);

	_evt_socket.set_filter(f);

	_notifier.fd(_evt_socket.handle());
	_notifier.can_read.connect( sigc::mem_fun( this, &DeviceManager::on_new_event ));
	_notifier.flags(POLLIN);

	/*	find devices
	*/
	struct
	{
		u16 ndevs;
		hci_dev_req devs[HCI_MAX_DEV];
	} list_req;

	list_req.ndevs = HCI_MAX_DEV;

	if (!ioctl(_evt_socket.handle(), HCIGETDEVLIST, (void *) &list_req))
	{
		for( int i = 0; i < list_req.ndevs; ++i )
		{
			int id = list_req.devs[i].dev_id;
			BdAddr addr = __get_dev_addr(id);
			_devices[id] = new Device(id,addr);
		}
	}
}

void DeviceManager::ListDevices( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	DBus::MessageIter rw = reply.w_iter();

	DevicePTable::iterator i = _devices.begin();
	while( i != _devices.end() )
	{
		const char* fullname = i->second->oname().c_str();
		rw.append_string(fullname);
		++i;
	}
	reply.append(DBUS_TYPE_INVALID);
	conn().send(reply);
}

void DeviceManager::EnableDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		DBus::ReturnMessage reply (msg);

		Hci::LocalDevice::up(hci_devid(name));

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}

void DeviceManager::DisableDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		DBus::ReturnMessage reply (msg);

		Hci::LocalDevice::down(hci_devid(name));

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}

void DeviceManager::ResetDevice( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	const char* error;
	u16 status;

	try
	{
		DBus::MessageIter ri = msg.r_iter();
		const char* name = ri.get_string();

		Hci::LocalDevice::reset(hci_devid(name));

		status = 0;
		reply.append(DBUS_TYPE_UINT16, &status, DBUS_TYPE_INVALID);
	}
	catch( std::exception& e )
	{
		status = 1;
		error = e.what();

		reply.append( DBUS_TYPE_UINT16, &status,
			      DBUS_TYPE_STRING, &error,
			      DBUS_TYPE_INVALID
		);
	}
	conn().send(reply);
}


Device* DeviceManager::get_device( int dev_id )
{
	DevicePTable::iterator i = _devices.find(dev_id);
	if( i != _devices.end() )
	{
		return i->second;	
	}
	return NULL;
}

void DeviceManager::on_new_event( FdNotifier& fn )
{
	char buf[HCI_MAX_FRAME_SIZE];

	int len = _evt_socket.read(buf,sizeof(buf));

	if(len < 0)	throw Hci::Exception();

	struct __hp
	{
		u8 type;
		hci_event_hdr eh;
		evt_stack_internal si;
	} __PACKED;

	__hp* hp = (__hp*)buf;

	if
	(     hp->type != HCI_EVENT_PKT
	   || hp->eh.evt != EVT_STACK_INTERNAL
	   || hp->si.type != EVT_SI_DEVICE
	)
	return;

	evt_si_device* sd = (evt_si_device*) &(hp->si.data);

	Device* dev = get_device(sd->dev_id);

	switch( sd->event )
	{
		case HCI_DEV_REG:
		{
			if(dev) return;

			Hci::LocalDevice::up(sd->dev_id);

			BdAddr addr = __get_dev_addr(sd->dev_id);
			_devices[sd->dev_id] = new Device(sd->dev_id,addr);
			break;
		}
		case HCI_DEV_UNREG:
		{
			if(!dev) return;

			DevicePTable::iterator i = _devices.find(sd->dev_id);

			delete i->second;
			_devices.erase(i);
			break;
		}
		case HCI_DEV_UP:
		{
			if(!dev) return;

			//dev->up();
			break;
		}
		case HCI_DEV_DOWN:
		{
			if(!dev) return;

			//dev->down();
			break;
		}
	}
}

}//namespace Bluetool
