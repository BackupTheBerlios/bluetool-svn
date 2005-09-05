#include <bluetool/sdperror.h>

#include <sys/errno.h>

namespace Sdp
{

Error::Error()
: Dbg::Error( strerror(errno) ), _code(errno)
{}

Error::Error( u16 code )
: Dbg::Error( strerror(code) ), _code(errno)
{}

Error::Error( const char* what )
: Dbg::Error( what ), _code(0)
{}

}
