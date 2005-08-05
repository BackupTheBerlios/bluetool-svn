#ifndef __BTOOL_HCI_DEVICE_H
#define __BTOOL_HCI_DEVICE_H

#include <map>
#include <string>

#include "../../cbus/cbus.h"
#include "../../libbluetool/hcidevice.h"
#include "../../libbluetool/hcievent.h"

#define DBUS_HCI_SERVICE	"org.bluetool.hci"
#define DBUS_HCIDEV_IFACE	"org.bluetool.hci.device"
#define DBUS_HCIDEV_PATH	"/org/bluetool/hci/"//+ hciX

class HciDevice;
typedef std::map<std::string, HciDevice*>	HciDevicePTable;

class HciDevice : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	HciDevice( std::string iface_name );

	/*	exported methods
	*/
	void GetProperty	( const DBus::CallMessage& );
	void SetProperty	( const DBus::CallMessage& );

	void StartInquiry	( const DBus::CallMessage& );

private:
	void on_hci_event( const Hci::EventPacket&, void* cookie, bool timedout );

private:
	Hci::LocalDevice	_device;
};

#endif//__BTOOL_HCI_DEVICE_H
