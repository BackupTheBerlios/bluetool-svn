#include "../sdperror.h"

#include <sys/errno.h>

namespace Sdp
{

Error:Error()
: _code(errno)
{}

const char* Error::what() const throw()
{
	return strerror(_code);
}

}
