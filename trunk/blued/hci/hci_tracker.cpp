#include "../bluedebug.h"

#include <exception>

#include "hci_tracker.h"

#define USE_HAL_DETECTION

HciTracker::HciTracker( /*HciDevicePTable& devices*/ )
//:	_devices(devices)
{
#ifdef USE_HAL_DETECTION
	/*	list for available devices
	
		TODO:	this will be done using the hal database
			as soon as I get it to work
	*/
 	HalDevices devs = FindDeviceByCapability("bluetooth_hci");
	HalDevices::iterator hi = devs.begin();
	while( hi != devs.end() )
	{
		const char* iface = (*hi)->GetPropertyString("bluetooth_hci.interface_name");
		if(!iface)
		{
			blue_dbg("AARRGH! Hal doesn't want to give us the interface name!");
		}
		_devices[iface] = new HciDevice( iface );
	
		delete *hi; // :(
		++hi;
	}

#else
//	_devices.clear();

	
 	update_table();
#endif
	
}

HciTracker::~HciTracker()
{
//todo: cleanup devices
}


void HciTracker::DeviceAdded( const DBus::SignalMessage& msg )
{
	DBus::MessageIter iter = msg.r_iter();
	const char* udi = iter.get_string();

 	blue_dbg("Added dev %s", udi);

#ifdef USE_HAL_DETECTION

	HalDevice dev(udi);

	/*	update the device list, if necessary
	*/
	
	if(dev.QueryCapability("bluetooth_hci"))	//TODO: this SEGFAULTS!!!
	
 	{
	
 		blue_dbg("--> YEAH <--");
	
 /*		const char* name = dev.GetPropertyString("bluetooth_hci.interface_name");
	
 		if( name )
 		{
 			blue_dbg("added device %s",name);
		}
	
 		else
 		{
 			blue_dbg("warning, unable to get HCI device name");
		}
*/
	
 	}

#else
	
 	//update_table();
#endif
	
}

void HciTracker::DeviceRemoved( const DBus::SignalMessage& )
{
	/*	update the device list, if necessary
	*/
	//update_table();
}
#if 0
static bool __find_table( HciDevicePTable& table, int id )
{
	HciDevicePTable::iterator it = table.begin();
	while(it != table.end())
	{
		if(it->first == id)
		
			return true;

	
		++it;
	}
	return false;
}

static bool __find_local( const Hci::LocalDevices& locals, int id )
{
	Hci::LocalDevices::const_iterator il = locals.begin();
	while(il != locals.end())
	{
	
		if( il->id() == id )
		
			return true;
		++il;
	}
	return false;
}

void HciTracker::update_table()
{
	Hci::LocalDevices locals = Hci::LocalDevice::enumerate();

	Hci::LocalDevices::iterator il = locals.begin();
	while(il != locals.end())
	{
		if(!__find_table(_devices, il->id()))

		{
		 	blue_dbg("Hci Device Added");

	
			//new device, cool!
			_devices[il->id()] = new HciDevice(*il);
		}
		++il;
	}


	HciDevicePTable::iterator it = _devices.begin();
	while(it != _devices.end())
	{
		if(!__find_local(locals,it->first))
		{
		 	blue_dbg("Hci Device Removed");

	
			//device was removed, sorry man
			delete it->second;
			_devices.erase(it);
		}
		++it;
	}


	
}
#endif
