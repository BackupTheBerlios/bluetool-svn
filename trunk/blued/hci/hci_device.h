#ifndef __BTOOL_HCI_DEVICE_H
#define __BTOOL_HCI_DEVICE_H

#include <map>
#include <string>
#include <algorithm>

#include <sys/time.h>

#include "../btool_common.h"

#include "../../cbus/cbus.h"
#include "../../libbluetool/hcidevice.h"
#include "../../libbluetool/hcievent.h"

#define	HCI_TIMEOUT 120000

class HciDevice;
typedef std::map<std::string, HciDevice*>	HciDevicePTable;

class HciRemote;
typedef std::map<std::string, HciRemote*>	HciRemotePTable;

class HciConnection;
typedef std::map<u16, HciConnection*>		HciConnPTable;


/*	local HCI device
*/
class HciDevice : public DBus::LocalInterface, public DBus::LocalObject
{
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
	void on_hci_event( const Hci::EventPacket&, void* cookie, bool timedout );

	void handle_command_complete( const Hci::EventPacket&, DBus::ReturnMessage* rpl );

	void clear_cache();
	void update_cache( const BdAddr&, u8, u8, u16 );
	void finalize_cache();

private:
	Hci::LocalDevice	_device;
	HciRemotePTable		_inquiry_cache;

	double			_time_last_inquiry;

friend class HciRemote;
friend class HciConnection;
};

/*	remote Bluetooth device
*/
class HciRemote : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	HciRemote( HciDevice& parent,
		   const BdAddr& addr,
		   u8 pscan_rpt_mode,
		   u8 pscan_mode,
		   u16 clk_offset );

	/*	exported methods
	*/
	void GetProperty	( const DBus::CallMessage& );
	void SetProperty	( const DBus::CallMessage& );

	/*	class methods
	*/
	void update( u8 pscan_rpt_mode, u8 pscan_mode, u16 clk_offset );
	double last_updated() const;

private:
	Hci::RemoteDevice	_device;
	HciConnPTable		_connections;

	double			_time_last_update;
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
