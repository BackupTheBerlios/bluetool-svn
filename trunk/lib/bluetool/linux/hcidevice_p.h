#ifndef __HCI_DEVICE_P_H
#define __HCI_DEVICE_P_H

#include "../hcidebug.h"
#include "../hcidevice.h"

#include <cstring>

#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

namespace Hci
{

/* internal representation of a hci request
*/

struct Request;
typedef std::list< RefPtr<Request> > Requests;

/*
*/

#define TVLESS(a,b) ((a).tv_sec == (b).tv_sec ? \
		    ((a).tv_usec < (b).tv_usec) : \
		    ((a).tv_sec < (b).tv_sec))

struct LocalDevice::Private
{
	void init();
	//void open( int dev_id );

	void post_req( RefPtr<Request>& );
	void fire_event( RefPtr<Request>& );

	void read_ready( FdNotifier& );
	void write_ready( FdNotifier& );

	void req_timedout( Timeout& );
	void flush_queues();

	void link_ctl_cmd_complete( Request& );

	void link_policy_cmd_complete( Request& );

	void host_ctl_cmd_complete( Request& );

	void info_param_cmd_complete( Request& );

	void status_param_cmd_complete( Request& );

	void hci_event_received( Request& );

	void hci_event_inquiry_result( u8 nrsp, inquiry_info* evt );
	void hci_event_conn_complete( evt_conn_complete* evt );
	void hci_event_disconn_complete( evt_disconn_complete* evt );

	void clear_cache();
	void update_cache( RemoteInfo& );
	void finalize_cache();

	/**/

	Private( LocalDevice*, int );
	~Private();

	/**/

	BdAddr	ba;
	Socket	dd;
	int	id;

	FdNotifier*	notifier;
	Requests	dispatchq;
	Requests	waitq;

	LocalDevice*	parent;

	RemoteDevPTable	inquiry_cache;
	timeval		time_last_inquiry;

	/**/

	void*		data;	//a wrapper can put here whatever he wants
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

	Timeout* to;

	enum
	{	QUEUED,		
		WRITING,
		WAITING,
		TIMEDOUT,
		COMPLETE
	} status;

	/*enum
	{
		LOCAL,
		REMOTE,
		CONNECTION,
	} dest_type;*/


	union
	{
		RemoteDevice*	remote;
		Connection*	conn;
	} src;

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
