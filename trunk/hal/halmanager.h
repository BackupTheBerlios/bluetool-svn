#ifndef __HAL_MANAGER_H
#define __HAL_MANAGER_H

#include "../cbus/cbus.h"

class HalManager;

#include "haldevice.h"

#define HAL_PATH	"org.freedesktop.Hal"
#define HALMAN_PATH	"/org/freedesktop/Hal/Manager"
#define HALMAN_IFACE	"org.freedesktop.Hal.Manager"

class HalManager : public DBus::RemoteInterface, public DBus::RemoteObject
{
public:
	HalManager();

	/*	remote methods
	*/
	HalDevices FindDeviceByCapability( const char* capability );

	/*	remote signals
	*/
	virtual void DeviceAdded( const DBus::SignalMessage& )
	{}

	virtual void DeviceRemoved( const DBus::SignalMessage& )
	{}

//	void	NewCapability( const DBus::SignalMessage& );

};

#endif//__HAL_BT_LISTENER_H
