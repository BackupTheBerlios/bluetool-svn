/*
 *
 *  C-Bus - C++ bindings for DBus
 *
 *  Copyright (C) 2005-2006  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


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

#include <common/fdnotifier.h>
#include <common/timeout.h>
#include <common/refptr.h>

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
