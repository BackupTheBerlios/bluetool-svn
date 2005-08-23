#ifndef __SOCKET_H
#define __SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

class Socket
{

public:

	Socket( int family, int proto, int type )
	: _af(family), _proto(proto), _type(type)
	{
		_fd = socket(_af, _proto, _type);
	}

	Socket( const Socket& sock )
	{
		_fd = dup(sock._fd);
	}

	virtual bool renew()
	{
		close();
		_fd = socket(_af, _proto, _type);
		return _fd < 0;
	}

	virtual ~Socket()
	{
		close();
	}

	int handle()
	{
		return _fd;
	}

	int close()
	{
		int f = _fd;
		_fd = -1;
		return ::close(f);
	}

	int send( const char* buf, int len, int flags = 0 )
	{
		return ::send(_fd, buf, len, flags);
	}

	int recv( char* buf, int len, int flags = 0 )
	{
		return ::recv(_fd, buf, len, flags);
	}

	int write( const char* buf, int len )
	{
		return ::write(_fd, buf, len);
	}

	int read( char* buf, int len )
	{
		return ::read(_fd, buf, len);
	}

	int setsockopt( int level, int optname, const char* optval, int optlen )
	{
		return ::setsockopt(_fd, level, optname, optval, optlen);
	}

	int flags()
	{
		return ::fcntl(_fd, F_GETFL, 0);
	}

	bool set_blocking( bool block )
	{
		int fl = block ? flags() | O_NONBLOCK : flags() & ~O_NONBLOCK;
		return ::fcntl(_fd, F_SETFL, fl) < 0;
	}

protected:

   	virtual bool bind( const struct sockaddr* sa, int sz )
	{
		return ::bind(_fd, sa, sz) == 0;
	}

	virtual bool connect( const struct sockaddr* sa, int sz )
	{
		return ::connect(_fd, sa, sz) == 0;
	}

private:

	int _fd;
	int _af;
	int _proto;
	int _type;
};

#endif//__SOCKET_H
