#include <cbus/cbuspendingcall.h>

namespace DBus
{

PendingCall::PendingCall( DBusPendingCall* c )
{
	_call = c;
}

PendingCall::PendingCall( const PendingCall& c )
{
	_call = c._call;
	ref();
}

PendingCall::~PendingCall()
{
	unref();
}

/*
00082 typedef void (* DBusPendingCallNotifyFunction) (DBusPendingCall *pending,
00083                                                 void            *user_data);
*/

void PendingCall::notify_stub( DBusPendingCall*, void* data )
{
	PendingCall* pc = static_cast<PendingCall*>(data);

	pc->completition(*pc);
}

}//namespace DBus
