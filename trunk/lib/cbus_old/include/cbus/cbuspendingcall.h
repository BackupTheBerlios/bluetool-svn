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


#ifndef __CBUS_PENDING_CALL_H
#define __CBUS_PENDING_CALL_H

#include <sigc++/sigc++.h>
#include <dbus/dbus.h>

namespace DBus
{
class PendingCall;

typedef sigc::signal<void,PendingCall&> Completition;
}

#include "cbusconnection.h"

/*
*/
namespace DBus
{

class PendingCall
{
public:
	PendingCall( const PendingCall& );

	virtual ~PendingCall();

	inline bool completed();
	
	inline void cancel();

	inline void block();

	Completition completition;

private:
	PendingCall( DBusPendingCall* );

	inline void ref();

	inline void unref();

	static void notify_stub( DBusPendingCall*, void* );

private:
	DBusPendingCall* _call;

friend class Connection;
};

/*
*/

bool PendingCall::completed()
{
	return dbus_pending_call_get_completed(_call);
}

void PendingCall::cancel()
{
	dbus_pending_call_cancel(_call);
}

void PendingCall::block()
{
	dbus_pending_call_block(_call);
}

void PendingCall::ref()
{	
	dbus_pending_call_ref(_call);
}

void PendingCall::unref()
{
	dbus_pending_call_unref(_call);
}

}//namespace DBus

#endif//__CBUS_PENDING_CALL_H
