#include "../hcidebug.h"
#include "../hciconnection.h"
#include "hcidevice_p.h"

namespace Hci
{

Connection::Connection
(
	RemoteDevice* to,
	ConnInfo& info
)
:	_to(to),
	_info(info)
{
}

Connection::~Connection()
{
/*	we don't care about disconnection
	we should be already disconnected
	when reaching this point
*/
}

}//namespace Hci
