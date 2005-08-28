#include <bluetool/sdperror.h>

#include <sys/errno.h>

namespace Sdp
{

Error::Error()
: _code(errno), _what(NULL)
{}

Error::Error( u16 code )
: _code(code), _what(NULL)
{}

Error::Error( const char* what )
: _code(0), _what(what)
{}

const char* Error::what() const throw()
{
	return _what ? _what : strerror(_code);
}

}
