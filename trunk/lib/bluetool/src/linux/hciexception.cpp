#include <bluetool/hciexception.h>

#include <errno.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <cstring>  //strerror()

namespace Hci
{

Exception::Exception( const char* error )
:	_code( 0 ), _error(error), _errno(errno)
{}

Exception::~Exception() throw()
{}

const char* Exception::what() const throw()
{
	
	return _error ? _error : strerror( _code ? bt_error(_code) : _errno );
}

}//namespace Hci
