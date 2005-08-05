#include <vector>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "../sdpdebug.h"
#include "../sdpsession.h"

namespace Sdp
{

typedef std::vector<u8> ByteVector;

struct Session::Private
{
	L2CapSocket sock;
	FdNotifier notifier;
	ByteVector buffer;

	sdp_pdu_hdr_t req;
	sdp_pdu_hdr_t ans;
	ByteVector cstate;

	Session* sess;
	bool busy;

	enum
	{
		INIT,
		CONNNECTED,
		WAITING,
		DISCONNECTED
	} state;

	void can_read( FdNotifier& );
	void can_write( FdNotifier& );
	void timed_out( Timeout& );
};

void Session::Private::can_read( FdNotifier& )
{
	switch( state )
	{
	};
}

void Session::Private::can_write( FdNotifier& )
{
	switch( state )
	{
	};
}


}//namespace Sdp
