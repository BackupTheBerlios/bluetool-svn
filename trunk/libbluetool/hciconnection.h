#ifndef __HCI_CONNECTION_H
#define __HCI_CONNECTION_H

#include "../common/types.h"

#include <map>

namespace Hci
{
	class Connection;

	typedef std::map<u16,Connection*> ConnPTable;
}

#include "hcidevice.h"

namespace Hci
{

class Connection
{
private:
	/* private ctor, this means you can't create a connection directly
	   but you have to ask the Hci::RemoteDevice to do that for you
	*/
	Connection
	(
		LocalDevice& from,
		RemoteDevice& to,
		u8 proto,
		u16 type,
		u8 encrypt,
		bool change_roles
	);

public:

	virtual ~Connection();

	/*	accessors
	*/
	inline LocalDevice& from();

	inline RemoteDevice& to();

	inline u16 handle();

	/*	connection operations
	*/
	void set_hold_mode( u16 max, u16 min, void* cookie, int timeout );

	void start_sniff_mode( /*TODO*/ void* cookie, int timeout );
	void end_sniff_mode( void* cookie, int timeout );

	void start_park_mode( u16 min, u16 max, void* cookie, int timeout );
	void end_park_mode( void* cookie, int timeout );

	void get_role( void* cookie, int timeout );
	void set_role( bool master, void* cookie, int timeout );

	void get_policy( void* cookie, int timeout );
	void set_policy( u16 policy, void* cookie, int timeout );

	void get_transmit_power( void* cookie, int timeout );
	
	void get_link_quality( void* cookie, int timeout );

	void get_rssi( void* cookie, int timeout );

	void disconnect( u8 reason, void* cookie, int timeout );

	/*	event handlers
	*/
	virtual void on_mode_change
	(
		u16 status,
		void* cookie,
		u8 mode
	){}

	virtual void on_get_role
	(
		u16 status,
		void* cookie,
		bool is_master
	){}

	virtual void on_role_change
	(
		u16 status,
		void* cookie,
		bool is_master
	){}

	virtual void on_get_policy
	(
		u16 status,
		void* cookie,
		u16 policy
	){}

	virtual void on_set_policy
	(
		u16 status,
		void* cookie
	){}

	virtual void on_get_link_quality
	(
		u16 status,
		void* cookie,
		u8 lq
	){}

	virtual void on_get_rssi
	(
		u16 status,
		void* cookie,
		u8 rssi
	){}

	virtual void on_get_transmit_power
	(
		u16 status,
		void* cookie,
		u8 power
	){}

	//bool set_filter( const Filter& flt );

public:
	
	//Event	on_event;

public:
	/*	commands
	*/

private:
	LocalDevice&	_from;
	RemoteDevice&	_to;
	u16		_handle;

friend class RemoteDevice;
};

LocalDevice& Connection::from()
{
	return _from;
}

RemoteDevice& Connection::to()
{
	return _to;
}

u16 Connection::handle()
{
	return _handle;
}

}//namespace Hci

#endif//__HCI_CONNECTION_H
