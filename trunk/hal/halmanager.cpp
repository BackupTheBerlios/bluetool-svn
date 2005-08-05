#include "haldebug.h"
#include "halmanager.h"

HalManager::HalManager()
:	
	/*	interface towards hal manager
	*/
	DBus::RemoteInterface( HALMAN_IFACE ),
	DBus::RemoteObject( "/org/freedesktop/Hal/Manager", DBus::Connection::SystemBus() )

{
	/*	signals from hal manager
	*/
	connect_signal(HalManager, DeviceAdded);
	connect_signal(HalManager, DeviceRemoved);
	//connect_signal(HalManager, NewCapability);
}

HalDevices HalManager::FindDeviceByCapability( const char* capability )
{

	
	HalDevices devices;

	
	try
	{

		/*	build request
		*/
		DBus::CallMessage msg = DBus::CallMessage(
						HAL_PATH,
						HALMAN_PATH,
						HALMAN_IFACE,
						"FindDeviceByCapability"
					);
	

		DBus::MessageIter iter = msg.w_iter();
		iter.append_string(capability);

		/*	invoke remote method
		*/
		DBus::Message reply = conn().send_blocking(msg, 2000);

	
		/*	wait for answers!
		*/
		DBus::MessageIter ri = reply.r_iter();

		if( ri.type() == DBUS_TYPE_ARRAY )
		{
			DBus::MessageIter rri = ri.recurse();

			while(!rri.at_end())
			{
				devices.push_back( new HalDevice( rri.get_string() ));
				++rri;
			}
		}

		hal_dbg("%d devices with %s capability returned by HAL", devices.size(), capability);

	}catch( exception& e )
	
	{
		hal_dbg("exception: %s", e.what());
	}

	return devices;
}
#if 0
void HalManager::NewCapability( const DBus::SignalMessage& msg )
{
	DBus::MessageIter iter = msg.iter_out();
	const char* udi = iter.get_string();
	const char* capability = iter.get_string();

	//HalDevice dev(udi);
	hal_dbg("device with udi %s has got capability %s", udi, capability);
}


void HalManager::DeviceAdded( const DBus::SignalMessage& msg )
{
	DBus::MessageIter iter = msg.iter_out();
	const char* udi = iter.get_string();

	HalDevice dev(udi);

	hal_dbg("added device with udi %s", udi);

	if(dev.HasCapability("bluetooth_hci"))
	{
		hal_dbg("---> hey man! it has bluetooth capability!!!");
		
		if(dev.PropertyExists("bluetooth_hci.inteface_name"))
		{
			const char* prop = dev.GetPropertyString("bluetooth_hci.interface_name");
			hal_dbg("and its name is  %s",prop);
		}
	}
}

void HalManager::DeviceRemoved( const DBus::SignalMessage& msg )
{
	hal_dbg("device removed");
}
#endif
