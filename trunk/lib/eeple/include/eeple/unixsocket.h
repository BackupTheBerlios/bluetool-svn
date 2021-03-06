#ifndef __EEPLE_SOCKET_H
#define __EEPLE_SOCKET_H

#include <sys/un.h>

#include "socket.h"

class UnixSocket : public Socket
{
public:
	UnixSocket()
	:	Socket(PF_UNIX,SOCK_STREAM,0)
	{}

	bool connect( const char* path )
	{
		sockaddr_un sa;
		sa.sun_family = AF_UNIX;
		strncpy(sa.sun_path, path, sizeof(sa.sun_path));

		return Socket::connect((sockaddr*)&sa, sizeof(sa));
	}
};

#endif//__EEPLE_SOCKET_H
