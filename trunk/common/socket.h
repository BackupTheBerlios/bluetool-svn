#ifndef __SOCKET_H
#define __SOCKET_H

#include  <sys/types.h>
#include  <sys/socket.h>
#include  <unistd.h>

class Socket
{

public:

	Socket( int family, int proto, int type )
	{
		_fd = socket(family, proto, type);
	}

	Socket( const Socket& sock )
	{
		_fd = dup(sock._fd);
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
		return ::close(_fd);
	}

	int send( const char* buf, int len, int flags = 0 )
	{
		return ::send(_fd, buf, len, flags);
	}

	int recv( char* buf, int len, int flags = 0 )
	{
		return ::recv(_fd, buf, len, flags);
	}

	int setsockopt( int level, int optname, const char* optval, int optlen )
	{
		return ::setsockopt(_fd, level, optname, optval, optlen);
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
};

#endif//__SOCKET_H
