#include "../hcidebug.h"
#include "../hciconnection.h"

namespace Hci
{

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

}//namespace Hci
