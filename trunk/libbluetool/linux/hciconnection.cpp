#include "../hcidebug.h"
#include "../hciconnection.h"
#include "hcidevice_p.h"

namespace Hci
{

#if 0

Connection::Connection( LocalDevice& from, RemoteDevice& to )
:	_from(from), _to(to), _dd(from.descriptor())
{
//	HciManager::connections.push_back(*this);
//	_iter = HciManager::connections.end();

//	_handle = ??
}

Connection::~Connection()
{
//	HciManager::connections.erase(_iter);
}

#endif

}//namespace Hci
