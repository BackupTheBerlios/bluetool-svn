#include <bluetool/hcierror.h>

#include <errno.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <cstring>  //strerror()

namespace Hci
{

Error::Error( u16 code )
: Dbg::Error( strerror( bt_error( code ) ) )
{}

Error::Error( const char* error )
: Dbg::Error( error ? error : strerror( errno ) )
{}

//Error::~Error() throw()
//{}

}//namespace Hci
