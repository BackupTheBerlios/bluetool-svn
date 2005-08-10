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

	}
	catch( std::exception& e )
	{
		hal_dbg("exception: %s", e.what());
	}

	return devices;
}
