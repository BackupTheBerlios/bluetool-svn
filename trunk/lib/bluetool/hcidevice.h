#ifndef __HCI_DEVICE_H
#define __HCI_DEVICE_H

#include <common/types.h>
#include <common/fdnotifier.h>
#include <common/timeout.h>
#include <common/refcnt.h>

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
	void up();
	void down();
	void reset();

	void on_up();
	void on_down();

public:

	LocalDevice( const char* dev_name );

	LocalDevice( int dev_id );

	virtual ~LocalDevice();

	Socket& descriptor();

	/*	direct accessors
	*/
	const BdAddr& addr() const;

	int id() const;

	int get_stats(int*,int*,int*,int*,int*);

	int get_link_mode( u32* );
	int set_link_mode( u32 );

	int get_link_policy( u32* );
	int set_link_policy( u32 );

	/*	device properties
	*/
	void get_auth_enable( void* cookie, int timeout );
	void set_auth_enable( u8, void* cookie, int timeout );

	void get_encrypt_mode( void* cookie, int timeout );
	void set_encrypt_mode( u8, void* cookie, int timeout );

	void get_scan_enable( void* cookie, int timeout );
	void set_scan_enable( u8, void* cookie, int timeout );

	void set_name( const char*, void* cookie, int timeout );
	void get_name( void* cookie, int timeout );

	void get_class( void* cookie, int timeout );
	void set_class( u8, u8, u8, void* cookie, int timeout );

	void get_voice_setting( void* cookie, int timeout );
	void set_voice_setting( u16 vs, void* cookie, int timeout );

	void get_address( void* cookie, int timeout );

	void get_version( void* cookie, int timeout );

	void get_features( void* cookie, int timeout );

	/*	device operations
	*/
	void start_inquiry( u8* lap, u32 flags, void* cookie );
	void cancel_inquiry( void* cookie, int timeout );

	void start_periodic_inquiry( u8* lap, u16 period, void* cookie, int timeout );
	void cancel_periodic_inquiry( void* cookie, int timeout );

private:
	/*	event handlers
	*/
	virtual void on_get_auth_enable
	(
		u16 status,
		void* cookie,
		u8 auth
	) = 0;

	virtual void on_set_auth_enable
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_encrypt_mode
	(
		u16 status,
		void* cookie,
		u8 encrypt
	) = 0;

	virtual void on_set_encrypt_mode
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_scan_enable
	(
		u16 status,
		void* cookie,
		u8 auth
	) = 0;

	virtual void on_set_scan_enable
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_name
	(
		u16 status,
		void* cookie,
		const char* name
	) = 0;

	virtual void on_set_name
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_class
	(
		u16 status,
		void* cookie,
		u8* dev_class
	) = 0;

	virtual void on_set_class
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_voice_setting
	(
		u16 status,
		void* cookie,
		u16 setting
	) = 0;

	virtual void on_set_voice_setting
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_get_address
	(
		u16 status,
		void* cookie,
		const char* address
	) = 0;

	virtual void on_get_version
	(
		u16 status,
		void* cookie,
		const char* hci_ver,
		u16 hci_rev,
		const char* lmp_ver,
		u16 lmp_subver,
		const char* manufacturer
	) = 0;

	virtual void on_get_features
	(
		u16 status,
		void* cookie,
		const char* features
	) = 0;

	virtual void on_inquiry_complete
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_inquiry_cancel
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_periodic_inquiry_started
	(
		u16 status,
		void* cookie
	) = 0;

	virtual void on_periodic_inquiry_cancel
	(
		u16 status,
		void* cookie
	) = 0;

	/*	special event handlers
	*/
	virtual void on_after_event( int error, void* cookie ) = 0;

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
	u8     dev_class[3];
	u8     pscan_rpt_mode;	//default is 0x02
	u8     pscan_per_mode;
	u8     pscan_mode;	//default is 0
	u16    clk_offset;	//default is 0
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

	inline LocalDevice* local();

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
	) = 0;

	virtual void on_get_version
	(
		u16 status,
		void* cookie,
		const char* lmp_ver,
		u16 lmp_sub,
		const char* manufacturer
	) = 0;

	virtual void on_get_features
	(
		u16 status,
		void* cookie,
		const char* features
	) = 0;

	virtual void on_get_clock_offset
	(
		u16 status,
		void* cookie,
		u16 clock_offset
	) = 0;

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

LocalDevice* RemoteDevice::local()
{
	return _local_dev;
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
