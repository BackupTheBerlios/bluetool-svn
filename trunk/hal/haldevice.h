#ifndef __HAL_DEVICE_H
#define __HAL_DEVICE_H

#include "../cbus/cbus.h"

#define HAL_PATH	"org.freedesktop.Hal"
#define HALDEV_PATH	"/org/freedesktop/Hal/Device"
#define HALDEV_IFACE	"org.freedesktop.Hal.Device"

#include <string>
#include <vector>

class HalDevice;

typedef vector<HalDevice*> HalDevices;

class HalDevice : public DBus::RemoteInterface, public DBus::RemoteObject
{
public:
	
	HalDevice( const char* udi );

	/*	remote methods
	*/
	bool PropertyExists( const char* property );
	const char* GetPropertyString( const char* property );

	bool QueryCapability( const char* capability );

private:

	std::string	_udi;
};

#endif//__HAL_DEVICE_H
