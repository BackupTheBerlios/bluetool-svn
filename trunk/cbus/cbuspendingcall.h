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
