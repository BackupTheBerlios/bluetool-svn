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

	class RemoteDevice;

	struct RemoteInfo;
}

#include "bdaddr.h"
#include "hciconnection.h"
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
	static void up( int );
	static void down( int );
	static void reset( int );

public:

	LocalDevice( const char* dev_name );

	LocalDevice( int dev_id );

	virtual ~LocalDevice();

	Socket& descriptor();

	/*	direct accessors
	*/
	const BdAddr& addr() const;

	int id() const;

	/*	device properties
	*/
	void get_auth_enable( void* cookie, int timeout );
	void set_auth_enable( u8, void* cookie, int timeout );

	void get_encrypt_mode( void* cookie, int timeout );
	void set_encrypt_mode( u8, void* cookie, int timeout );

	void get_scan_type( void* cookie, int timeout );
	void set_scan_type( u8, void* cookie, int timeout );

	void set_name( const char*, void* cookie, int timeout );
	void get_name( void* cookie, int timeout );

	void get_class( void* cookie, int timeout );
	void set_class( u32 cls, void* cookie, int timeout );

	void get_voice_setting( void* cookie, int timeout );
	void set_voice_setting( u16 vs, void* cookie, int timeout );

	void get_address( void* cookie, int timeout );

	void get_version( void* cookie, int timeout );

	void get_features( void* cookie, int timeout );

	/*	device operations
	*/
	void start_inquiry( u8* lap, u32 flags, void* cookie );
	void cancel_inquiry( void* cookie );

private:
	/*	event handlers
	*/
	virtual void on_get_auth_enable
	(
		u16 status,
		void* cookie,
		u8 auth
	){}

	virtual void on_set_auth_enable
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_encrypt_mode
	(
		u16 status,
		void* cookie,
		u8 encrypt
	){}

	virtual void on_set_encrypt_mode
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_scan_type
	(
		u16 status,
		void* cookie,
		u8 auth
	){}

	virtual void on_set_scan_type
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_name
	(
		u16 status,
		void* cookie,
		const char* name
	){}

	virtual void on_set_name
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_class
	(
		u16 status,
		void* cookie,
		u8* dev_class
	){}

	virtual void on_set_class
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_voice_setting
	(
		u16 status,
		void* cookie,
		u16 setting
	){}

	virtual void on_set_voice_setting
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_address
	(
		u16 status,
		void* cookie,
		const char* address
	){}

	virtual void on_get_version
	(
		u16 status,
		void* cookie,
		const char* hci_ver,
		u16 hci_rev,
		const char* lmp_ver,
		u16 lmp_subver,
		const char* manufacturer
	){}

	virtual void on_get_features
	(
		u16 status,
		void* cookie,
		const char* features
	){}

	virtual void on_inquiry_complete
	(
		u16 status,
		void* cookie
	){}

	/*	special event handlers
	*/
	virtual void on_after_event( void* cookie ) = 0;

	virtual RemoteDevice* on_new_cache_entry( RemoteInfo& ) = 0;

public:
	struct Private;

private:	
	Private* pvt;

friend class RemoteDevice;
friend class Connection;	
};

/*
*/
struct RemoteInfo
{
	BdAddr addr;
	u8     pscan_rpt_mode;
	u8     pscan_per_mode;
	u8     pscan_mode;
	u8     dev_class[3];
	u16    clk_offset;
};

class RemoteDevice
{
public:

	RemoteDevice
	(
		LocalDevice* local_dev,
		RemoteInfo& info
	);

	virtual ~RemoteDevice();

	/*	direct accessors
	*/
	inline const BdAddr& addr() const;

	inline u8 page_scan_repeat_mode() const;
	inline void page_scan_repeat_mode( u8 );

	inline u8 page_scan_mode() const;
	inline void page_scan_mode( u8 );

	inline u16 clock_offset() const;
	inline void clock_offset( u8 );

	/*	device properties
	*/
	void get_name( void* cookie, int timeout );

	void get_version( void* cookie, int timeout );	//ACL connection needed

	void get_features( void* cookie, int timeout );	//ACL connection needed

	void get_clock_offset( void* cookie, int timeout ); //here too

	void create_acl_connection
	(
		u16 ptype,
		bool change_role,
		void* cookie,
		int timeout
	);

	void create_sco_connection
	(
		u16 acl_handle,
		u16 ptype,
		void* cookie,
		int timeout
	);

	/*	cache managment
	*/
	void update( RemoteInfo& );

	inline double last_updated() const;

private:
	/*	event handlers
	*/
	virtual void on_get_name
	(	
		u16 status,
		void* cookie,
		const char* name
	){}

	virtual void on_get_version
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_features
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_clock_offset
	(
		u16 status,
		void* cookie
	){}

	/*	special handlers
	*/
	virtual Hci::Connection* on_new_connection( ConnInfo& ) = 0;

	/* misc..
	*/
	bool remove_connection(u16);

	/*	special event handlers
	*/
	//virtual void on_after_event( void* cookie ) = 0;

private:

	RemoteInfo	_info;

	LocalDevice*	_local_dev;

	ConnPTable	_connections;

	double		_time_last_updated;

friend struct LocalDevice::Private;
};

const BdAddr& RemoteDevice::addr() const
{
	return _info.addr;
}

double RemoteDevice::last_updated() const
{
	return _time_last_updated;
}

u8 RemoteDevice::page_scan_repeat_mode() const
{
	return _info.pscan_rpt_mode;
}

void RemoteDevice::page_scan_repeat_mode( u8 prm )
{
	_info.pscan_rpt_mode = prm;
}

u8 RemoteDevice::page_scan_mode() const
{
	return _info.pscan_mode;
}
void RemoteDevice::page_scan_mode( u8 pm )
{
	_info.pscan_mode = pm;
}

u16 RemoteDevice::clock_offset() const
{
	return _info.clk_offset;
}
void RemoteDevice::clock_offset( u8 co )
{
	_info.clk_offset = co;
}

}//namespace Hci

#endif//__HCI_DEVICE_H
