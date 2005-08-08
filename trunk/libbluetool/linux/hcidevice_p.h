#ifndef __HCI_DEVICE_P_H
#define __HCI_DEVICE_P_H

#include "../hcidevice.h"

#include <cstring>

#include <sys/ioctl.h>
#include <sys/errno.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

namespace Hci
{

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

	/**/

	Socket	dd;
	int	id;

	FdNotifier	notifier;
	Requests	dispatchq;
	Requests	waitq;

	LocalDevice*	parent;
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
