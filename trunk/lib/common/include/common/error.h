#ifndef __ERROR_H
#define __ERROR_H

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

#endif//__ERROR_H
