#ifndef __CBUS_MONITOR_H
#define __CBUS_MONITOR_H

#include <list>
#include <algorithm>

#include <dbus/dbus.h>
#include <sys/poll.h>

namespace DBus
{
	class Monitor;
}

#include "../common/fdnotifier.h"
#include "../common/timeout.h"
#include "../common/refptr.h"

namespace DBus
{

class Monitor
{
public:
	Monitor();

	virtual ~Monitor();

protected:
	
	void init( DBusConnection* );

	void init( DBusServer* );

private:
	
	virtual void do_dispatch() = 0;

private:

	struct	Private;

//	Private* pvt;
	int pvtslot;

//	FdNotifierPList	_fdnotifiers;
//	TimeoutPList	_timeouts;
};

typedef std::list<Monitor*> MonitorPList;

}//namespace DBus

#endif//__CBUS_MONITOR_H
