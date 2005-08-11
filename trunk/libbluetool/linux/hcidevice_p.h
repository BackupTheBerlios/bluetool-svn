#ifndef __HCI_DEVICE_P_H
#define __HCI_DEVICE_P_H

#include "../hcidebug.h"
#include "../hcidevice.h"

#include <cstring>
#include <map>

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

namespace Hci
{

typedef std::map<std::string,RemoteDevice*> RemoteDevPTable;

/* internal representation of a hci request
*/

struct Request;
typedef std::list<Request*> Requests;

/*
*/

struct LocalDevice::Private
{
	void open( int dev_id );

	void post_req( Request* );
	void fire_event( Request* );

	void read_ready( FdNotifier& );
	void write_ready( FdNotifier& );

	void req_timedout( Timeout& );

	void link_ctl_cmd_complete( Request* req );

	void link_policy_cmd_complete( Request* req );

	void host_ctl_cmd_complete( Request* req );

	void info_param_cmd_complete( Request* req );

	void status_param_cmd_complete( Request* req );

	void hci_event_received( Request* req );

	void clear_cache();
	void update_cache( const BdAddr&, u8, u8, u16 );
	void finalize_cache();

	/**/

	BdAddr	ba;
	Socket	dd;
	int	id;

	FdNotifier	notifier;
	Requests	dispatchq;
	Requests	waitq;

	LocalDevice*	parent;

	RemoteDevPTable	inquiry_cache;
	double		time_last_inquiry;
};

/*
*/

struct Request
{
	/* hci request
	*/
	hci_request hr;

	u8 type;
	hci_command_hdr ch;

	iovec iobuf[3];
	int ion;

	Timeout to;

	enum
	{	QUEUED,		
		WRITING,
		WAITING,
		TIMEDOUT,
		COMPLETE
	} status;

	/* caller tracking
	*/
	void* cookie;
	
	/* queue position
	*/
	Requests::iterator iter;

	/* constructor
	*/
	Request();

	/* destructor
	*/
	~Request();
};


}//namespace Hci

#endif//__HCI_DEVICE_P_H
