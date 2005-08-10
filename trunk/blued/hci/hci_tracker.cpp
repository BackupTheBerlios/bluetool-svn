#include "../bluedebug.h"

#include <exception>

#include "hci_tracker.h"

HciTracker::HciTracker()
{
 	HalDevices devs = FindDeviceByCapability("bluetooth_hci");
	HalDevices::iterator hi = devs.begin();
	while( hi != devs.end() )
	{
		const char* iface = (*hi)->GetPropertyString("bluetooth_hci.interface_name");
		if(!iface)
		{
 			blue_dbg("unable to get HCI device name");
		}
		try
		{
			_devices[iface] = new HciDevice( iface );
		}
		catch( std::exception& e )
		{
			blue_dbg("error connecting to device %s: %s", iface, e.what());
		}
		delete *hi;
		++hi;
	}
}

HciTracker::~HciTracker()
{
	HciDevicePTable::iterator i = _devices.begin();

	while( i != _devices.end() )
	{
		delete i->second;
		++i;
	}
}


void HciTracker::DeviceAdded( const DBus::SignalMessage& msg )
{
	DBus::MessageIter iter = msg.r_iter();
	const char* udi = iter.get_string();

// 	blue_dbg("Added dev %s", udi);

	HalDevice dev(udi);

	/*	update the device list, if necessary
	*/
	if(dev.QueryCapability("bluetooth_hci"))
 	{	
 		blue_dbg("New device with bluetooth_hci capability (%s)",udi);
	
		const char* iface = dev.GetPropertyString("bluetooth_hci.interface_name");
 		if( !iface )
 		{
 			blue_dbg("unable to get HCI device name");
		}
		else
		{
			try
			{
				_devices[iface] = new HciDevice( iface );
			}
			catch( std::exception& e )
			{
				blue_dbg("error connecting to device %s: %s", iface, e.what());
			}
		}
 	}
}

void HciTracker::DeviceRemoved( const DBus::SignalMessage& )
{
	/*	TODO: update the device list, if necessary
	*/
}
