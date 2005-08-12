#include "hci_manager.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <sys/ioctl.h>

HciService::HciService()
:	_dbus_conn( DBus::Connection::SystemBus() ),
	
 	_hci_manager( _dbus_conn )
{
	/*	register HCI service
	*/
	_dbus_conn.request_name( DBUS_HCI_SERVICE );
}

/*
*/

HciManager::HciManager( DBus::Connection& conn )
:	DBus::LocalInterface	( DBUS_HCIMAN_IFACE ),
	DBus::LocalObject	( DBUS_HCIMAN_PATH, conn )
{
	/*	export all methods in the interface
	*/
	register_method( HciManager, ListDevices );
	register_method( HciManager, EnableDevice );
	register_method( HciManager, DisableDevice );
	register_method( HciManager, ResetDevice );

	/*
	*/
	if(!_evt_socket.bind(HCI_DEV_NONE))
		throw Hci::Exception();

	Hci::Filter f;
	f.set_event(EVT_STACK_INTERNAL);
	f.set_type(HCI_EVENT_PKT);

	_evt_socket.set_filter(f);

	_notifier.fd(_evt_socket.handle());
	_notifier.can_read.connect( sigc::mem_fun( this, &HciManager::on_new_event ));
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
			_devices[id] = new HciDevice(id);
		}
	}
}

void HciManager::ListDevices( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);
	DBus::MessageIter rw = reply.w_iter();

	HciDevicePTable::iterator i = _devices.begin();
	while( i != _devices.end() )
	{
		const char* fullname = i->second->oname().c_str();
		rw.append_string(fullname);
		++i;
	}
	reply.append(DBUS_TYPE_INVALID);
	conn().send(reply);
}

void HciManager::EnableDevice( const DBus::CallMessage& msg )
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

void HciManager::DisableDevice( const DBus::CallMessage& msg )
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

void HciManager::ResetDevice( const DBus::CallMessage& msg )
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


HciDevice* HciManager::get_device( int dev_id )
{
	HciDevicePTable::iterator i = _devices.find(dev_id);
	if( i != _devices.end() )
	{
		return i->second;	
	}
	return NULL;
}

void HciManager::on_new_event( FdNotifier& fn )
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

	HciDevice* dev = get_device(sd->dev_id);

	switch( sd->event )
	{
		case HCI_DEV_REG:
		{
			if(dev) return;

			_devices[sd->dev_id] = new HciDevice(sd->dev_id);
			break;
		}
		case HCI_DEV_UNREG:
		{
			if(!dev) return;

			HciDevicePTable::iterator i = _devices.find(sd->dev_id);

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

