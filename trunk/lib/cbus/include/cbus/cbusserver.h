#ifndef __CBUS_SERVER_H
#define __CBUS_SERVER_H

#include <list>

namespace DBus
{
class Server;

typedef std::list<Server> ServerList;
}

#include <common/eventloop.h>
#include "cbusmonitor.h"
#include "cbuserror.h"
#include "cbusconnection.h"

namespace DBus
{

class Server : public Monitor
{
public:
	Server( const char* address );

	Server( const Server& s );

	virtual ~Server();

	inline bool operator == ( const Server& ) const;

	inline bool listening() const;

	inline void disconnect();

protected:

	virtual void on_new_connection( Connection& c ) = 0;

private:

	inline void ref();

	inline void unref();

	void do_dispatch();

private:

	static void on_new_conn_cb( DBusServer* server, DBusConnection* conn, void* param );

private:

	DBusServer* _server;

friend class EventLoop;
};

/*
*/

bool Server::operator == ( const Server& s ) const
{
	return _server == s._server;
}

bool Server::listening() const
{
	return dbus_server_get_is_connected(_server);
}
void Server::disconnect()
{
	dbus_server_disconnect(_server);
}
void Server::ref()
{
	dbus_server_ref(_server);
}
void Server::unref()
{
	dbus_server_ref(_server);
}

}//namespace DBus

#endif//__CBUS_SERVER_H
