#ifndef __HCI_EXCEPTION_H
#define __HCI_EXCEPTION_H

#include <exception>
#include <cstdlib>
#include <common/types.h>

namespace Hci
{

class Exception : public std::exception
{

public:

	Exception( u16 code )
	: _code(code), _error(NULL), _errno(0)
	{}

	Exception( const char* error = NULL );

	~Exception() throw();

	const char* what() const throw();

private:

	u16		_code;
	const char*	_error;
	int		_errno;
};

}//namespace Hci

#endif//__HCI_EXCEPTION_H
