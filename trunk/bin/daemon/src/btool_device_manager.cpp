#include "bluedebug.h"
#include "btool_device_manager.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <sys/ioctl.h>

#include <iostream>

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
	DBus::LocalObject    ( BTOOL_DEVMAN_PATH, DBus::Connection::SystemBus() ),

	_services( this->oname(), std::string("") )
{
	/*	export all methods in the interface
	*/
	register_method( DeviceManager, ListDevices );
//	register_method( DeviceManager, EnableDevice );
//	register_method( DeviceManager, DisableDevice );
//	register_method( DeviceManager, ResetDevice );

	/*
	*/
	if(!_evt_socket.bind(HCI_DEV_NONE))
		throw Dbg::Errno();

	Hci::Filter f;
	f.set_event(EVT_STACK_INTERNAL);
	f.set_type(HCI_EVENT_PKT);

	_evt_socket.set_filter(f);

	_notifier = FdNotifier::create( _evt_socket.handle() );
	_notifier->can_read.connect( sigc::mem_fun( this, &DeviceManager::on_new_event ));
	_notifier->flags(POLLIN);

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

			try
			{
				bool up = Device::is_up( id );
				if(!up)
					Device::up( id );

				RefPtr<Device> rd ( new Device(id) );

				if(up)
					rd->on_up();

				_devices[id] = rd;

				this->DeviceAdded(rd->oname().c_str());
			}
			catch( std::exception& e )
			{
				blue_dbg("Unable to create device #%d: %s",id,e.what());
			}
		}
	}
}

DeviceManager::~DeviceManager()
{
	FdNotifier::destroy(_notifier);

	DeviceRTable::iterator i = _devices.begin();
	while( i != _devices.end() )
	{
		DeviceRemoved(i->second->oname().c_str());
		_devices.erase(i);
		++i;
	}
}

void DeviceManager::ListDevices( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	DBus::MessageIter rw = reply.w_iter();
	DBus::MessageIter sa = rw.new_array(DBUS_TYPE_STRING);

	DeviceRTable::iterator i = _devices.begin();
	while( i != _devices.end() )
	{
		const char* fullname = i->second->oname().c_str();
		sa.append_string(fullname);
		++i;
	}
	rw.close_container(sa);
	reply.append(DBUS_TYPE_INVALID);
	conn().send(reply);
}
#if 0
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
#endif
void DeviceManager::DeviceAdded( const char* name )
{
	DBus::SignalMessage sig (oname().c_str(),iname().c_str(),"DeviceAdded");

	sig.append(DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

	conn().send(sig);
}

void DeviceManager::DeviceRemoved( const char* name )
{
	DBus::SignalMessage sig (oname().c_str(),iname().c_str(),"DeviceRemoved");

	sig.append(DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

	conn().send(sig);
}

void DeviceManager::DeviceUp( const char* name )
{
	DBus::SignalMessage sig (oname().c_str(),iname().c_str(),"DeviceUp");

	sig.append(DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

	conn().send(sig);
}

void DeviceManager::DeviceDown( const char* name )
{
	DBus::SignalMessage sig (oname().c_str(),iname().c_str(),"DeviceDown");

	sig.append(DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

	conn().send(sig);
}

Device* DeviceManager::get_device( int dev_id )
{
	DeviceRTable::iterator i = _devices.find(dev_id);
	if( i != _devices.end() )
	{
		return &(*i->second);	
	}
	return NULL;
}

void DeviceManager::on_new_event( FdNotifier& fn )
{
	char buf[HCI_MAX_FRAME_SIZE];

	int len = _evt_socket.read(buf,sizeof(buf));

	if(len < 0)	throw Dbg::Errno();

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

			try
			{
				//usleep(1000000); //it seems the device doesn't come up immediately sometimes :(

				bool up = Device::is_up( sd->dev_id );
				if(!up)
					Device::up( sd->dev_id );

				RefPtr<Device> rd ( new Device(sd->dev_id) );

				if(up)
					rd->on_up();

				_devices[sd->dev_id] = rd;

				this->DeviceAdded(rd->oname().c_str());
			}
			catch( std::exception& e )
			{
				blue_dbg("Unable to create device #%d: %s",sd->dev_id,e.what());
			}

			break;
		}
		case HCI_DEV_UNREG:
		{
			if(!dev) return;

			DeviceRTable::iterator i = _devices.find(sd->dev_id);

			std::string oldname = i->second->oname().c_str();

			_devices.erase(i);

			this->DeviceRemoved(oldname.c_str());

			break;
		}
		case HCI_DEV_UP:
		{
			if(!dev) return;
			
			try
			{
				dev->on_up();

				this->DeviceUp(dev->oname().c_str());
			}
			catch( std::exception& e )
			{
				blue_dbg("Unable to initialize device #%d: %s",sd->dev_id,e.what());
			}

			break;
		}
		case HCI_DEV_DOWN:
		{
			if(!dev) return;

			this->DeviceDown(dev->oname().c_str());

			dev->on_down();
			break;
		}
	}
}

}//namespace Bluetool
