#include <cbus/cbusserver.h>

namespace DBus
{

Server::Server( const char* address )
{
	Error e;
	_server = dbus_server_listen(address, e);

	if(e)	throw e;

	Monitor::init(_server);
}

Server::Server( const Server& s )
{
	_server = s._server;
	ref();
}

Server::~Server()
{
	unref();
}

}//namespace DBus
