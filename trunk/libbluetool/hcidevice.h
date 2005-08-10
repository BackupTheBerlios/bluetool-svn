#ifndef __HCI_DEVICE_H
#define __HCI_DEVICE_H

#include "../common/types.h"
#include "../common/fdnotifier.h"
#include "../common/timeout.h"
#include "../common/refcnt.h"

#include <string>
#include <list>

namespace Hci
{
	class LocalDevice;
	typedef std::list<LocalDevice*> LocalDevPList;

	class RemoteDevice;
	typedef std::list<RemoteDevice*> RemoteDevPList;
}

#include "bdaddr.h"
#include "hciconnection.h"
#include "hcievent.h"
#include "hcisocket.h"
#include "hciexception.h"

namespace Hci
{

class LocalDevice : public RefCnt
{
public:

/*	device enumeration
*/	
	//static LocalDevices enumerate();

/*	device control
*/	
	static void up( const char* name );
	static void down( const char* name );
	static void reset( const char* name );

public:

	LocalDevice( const char* dev_name );

	LocalDevice( int dev_id );

	virtual ~LocalDevice();

	Socket& descriptor();

	int id() const;

/*	device properties
*/
	void get_auth_enable( void* cookie, int timeout );
	void set_auth_enable( u8, void* cookie, int timeout );

	void get_encrypt_mode( void* cookie, int timeout );
	void set_encrypt_mode( u8, void* cookie, int timeout );

	void get_scan_type( void* cookie, int timeout );
	void set_scan_type( u8, void* cookie, int timeout );

	void set_local_name( const char*, void* cookie, int timeout );
	void get_local_name( void* cookie, int timeout );

	void get_class( void* cookie, int timeout );
	void set_class( u32 cls, void* cookie, int timeout );

	void get_voice_setting( void* cookie, int timeout );
	void set_voice_setting( u16 vs, void* cookie, int timeout );

	void get_version( void* cookie, int timeout );

	void get_features( void* cookie, int timeout );

	void get_addr( void* cookie, int timeout );

/*	device operations
*/
	void start_inquiry( u8* lap, u32 flags, void* cookie );
	void cancel_inquiry( void* cookie );
public:
	
	Event	on_event;

private:	

	struct Private;
	Private* pvt;	
};

/*
*/

class RemoteDevice
{
public:

	RemoteDevice(
		const BdAddr& addr,
		u8 pscan_rpt_mode,
		u8 pscan_mode,
		u16 clk_offset
	);

	virtual ~RemoteDevice();

	inline const BdAddr& addr();

public:

	Event	on_event;

public:
	/*	commands
	*/

private:

	BdAddr          _addr;
	u8		_pscan_rpt_mode;
	u8		_pscan_mode;
	u16		_clk_offset;
	Connection* 	_acl_conn;

};

const BdAddr& RemoteDevice::addr()
{
	return _addr;
}

}//namespace Hci

#endif//__HCI_DEVICE_H
