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

	/*	special event handlers
	*/
	virtual void on_after_event( void* cookie )
	{}


	/*	special handlers
	*/
	virtual RemoteDevice* on_new_remote
	(
		LocalDevice* local_dev,
		const BdAddr& addr,
		u8 pscan_rpt_mode,
		u8 pscan_mode,
		u16 clk_offset
	)
	{
		return NULL;
	}

private:	

	struct Private;
	Private* pvt;	
};

/*
*/

class RemoteDevice
{
public:

	RemoteDevice
	(
		LocalDevice* local_dev,
		const BdAddr& addr,
		u8 pscan_rpt_mode,
		u8 pscan_mode,
		u16 clk_offset
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

	void get_version( void* cookie, int timeout );	//needs a connection

	void get_features( void* cookie, int timeout );	//needs a connection

	void get_clock_offset( void* cookie, int timeout );

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

/*	cache managment
*/
	void update(u8,u8,u16);

	inline double last_updated() const;

private:

	BdAddr          _addr;
	u8		_pscan_rpt_mode;
	u8		_pscan_mode;
	u16		_clk_offset;
	LocalDevice*	_local_dev;
	double		_time_last_updated;
};

const BdAddr& RemoteDevice::addr() const
{
	return _addr;
}

double RemoteDevice::last_updated() const
{
	return _time_last_updated;
}

u8 RemoteDevice::page_scan_repeat_mode() const
{
	return _pscan_rpt_mode;
}

void RemoteDevice::page_scan_repeat_mode( u8 prm )
{
	_pscan_rpt_mode = prm;
}

u8 RemoteDevice::page_scan_mode() const
{
	return _pscan_mode;
}
void RemoteDevice::page_scan_mode( u8 pm )
{
	_pscan_mode = pm;
}

u16 RemoteDevice::clock_offset() const
{
	return _clk_offset;
}
void RemoteDevice::clock_offset( u8 co )
{
	_clk_offset = co;
}

}//namespace Hci

#endif//__HCI_DEVICE_H
