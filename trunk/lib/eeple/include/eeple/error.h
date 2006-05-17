#ifndef __EEPLE_ERROR_H
#define __EEPLE_ERROR_H

#include <cstring>
#include <exception>

#include <errno.h>

namespace Dbg
{

class Error : public std::exception
{
public:
	Error( const char* what ) : _what(what)
	{}

	Error( int code ) : _what(strerror(code))
	{}

	~Error() throw()
	{}

	virtual const char* what() const throw()
	{
		return _what;
	}

private:
	const char* _what;
};

class Errno : public Error
{
public:
	Errno() : Error( strerror(errno) )
	{}
};

}

#endif//__EEPLE_ERROR_H
