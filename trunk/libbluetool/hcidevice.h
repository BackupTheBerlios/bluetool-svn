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
	typedef std::list<LocalDevice> LocalDevices;

	class RemoteDevice;
	typedef std::list<RemoteDevice> RemoteDevices;
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
	
	static LocalDevices enumerate();

public:

	LocalDevice( const char* dev_name );

	LocalDevice( int dev_id );

	virtual ~LocalDevice();

	Socket& descriptor();

	int id() const;

/*	device control
*/	
	void up();
	void down();
	void reset();

/*	device properties
*/
	void auth_enable( bool );
	bool auth_enable();

	void encrypt_enable( bool );
	bool encrypt_enable();

	void secman_enable( bool );
	bool secman_enable();

	void pscan_enable( bool );
	bool pscan_enable();

	void iscan_enable( bool );
	bool iscan_enable();

	void local_name( const char* );
	const char* local_name();

/*	device operations
*/
	void start_inquiry( u8* lap, u32 flags, void* cookie );
	void cancel_inquiry();
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
