#ifndef __BTOOL_HCI_DEVICE_H
#define __BTOOL_HCI_DEVICE_H

#include <map>
#include <string>
#include <algorithm>

#include "../btool_common.h"

#include "../../cbus/cbus.h"
#include "../../libbluetool/hcidevice.h"
#include "../../libbluetool/bdaddr.h"

#define	HCI_TIMEOUT 120000

class HciDevice;
typedef std::map<std::string, HciDevice*>	HciDevicePTable;

class HciRemote;
//typedef std::map<std::string, HciRemote*>	HciRemotePTable;

class HciConnection;
typedef std::map<u16, HciConnection*>		HciConnPTable;


/*	local HCI device
*/
class HciDevice : private Hci::LocalDevice, public DBus::LocalInterface, public DBus::LocalObject
{					//todo, split into different interfaces
public:
	HciDevice( std::string iface_name );
	~HciDevice();

	/*	exported methods
	*/
	void GetProperty	( const DBus::CallMessage& );
	void SetProperty	( const DBus::CallMessage& );

	void StartInquiry	( const DBus::CallMessage& );
	void CancelInquiry	( const DBus::CallMessage& );
	void PeriodicInquiry	( const DBus::CallMessage& );
	void ExitPeriodicInquiry( const DBus::CallMessage& );
	void InquiryCache	( const DBus::CallMessage& );

	void CreateConnection	( const DBus::CallMessage& );

	/*	signals
	*/
	void DeviceInRange	( const HciRemote& );
	void DeviceOutOfRange	( const HciRemote& );

private:

	/*	event handlers
	*/
	void on_get_auth_enable
	(
		u16 status,
		void* cookie,
		u8 auth
	);

	void on_set_auth_enable
	(
		u16 status,
		void* cookie
	);

	void on_get_encrypt_mode
	(
		u16 status,
		void* cookie,
		u8 encrypt
	);

	void on_set_encrypt_mode
	(
		u16 status,
		void* cookie
	);

	void on_get_scan_type
	(
		u16 status,
		void* cookie,
		u8 auth
	);

	void on_set_scan_type
	(
		u16 status,
		void* cookie
	);

	void on_get_name
	(
		u16 status,
		void* cookie,
		const char* name
	);

	void on_set_name
	(
		u16 status,
		void* cookie
	);

	void on_get_class
	(
		u16 status,
		void* cookie,
		u8* dev_class
	);

	void on_set_class
	(
		u16 status,
		void* cookie
	);

	void on_get_voice_setting
	(
		u16 status,
		void* cookie,
		u16 setting
	);

	void on_set_voice_setting
	(
		u16 status,
		void* cookie
	);

	void on_get_address
	(
		u16 status,
		void* cookie,
		const char* address
	);

	void on_get_version
	(
		u16 status,
		void* cookie,
		const char* hci_ver,
		u16 hci_rev,
		const char* lmp_ver,
		u16 lmp_subver,
		const char* manufacturer
	);

	void on_get_features
	(
		u16 status,
		void* cookie,
		const char* features
	);

	void on_inquiry_complete
	(
		u16 status,
		void* cookie
	);

	/*	special handlers
	*/
	void on_after_event( void* cookie );

	Hci::RemoteDevice* on_new_cache_entry( Hci::RemoteInfo& );

friend class HciRemote;
friend class HciConnection;
};

/*	remote Bluetooth device
*/
class HciRemote : public Hci::RemoteDevice, public DBus::LocalInterface, public DBus::LocalObject
{
public:
	HciRemote
	( 
		const char* obj_name,
		Hci::LocalDevice* parent,
		Hci::RemoteInfo& info
	);

	~HciRemote();

	/*	exported methods
	*/
	void GetProperty	( const DBus::CallMessage& );
	void SetProperty	( const DBus::CallMessage& );

	/*	event handlers
	*/
	void on_get_name
	(	
		u16 status,
		void* cookie,
		const char* name
	);

	void on_get_version
	(
		u16 status,
		void* cookie
	);

	void on_get_features
	(
		u16 status,
		void* cookie
	);

	void on_get_clock_offset
	(
		u16 status,
		void* cookie
	);

private:
	HciConnPTable		_connections;
};

/*	HCI Connection
*/
class HciConnection : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	HciConnection();

	/*	exported methods
	*/
	void GetProperty	( const DBus::CallMessage& );
	void SetProperty	( const DBus::CallMessage& );

private:
	Hci::Connection		_connection;
};

#endif//__BTOOL_HCI_DEVICE_H
