#include <cbus/cbusdebug.h>
#include <cbus/cbusserver.h>

namespace DBus
{

void Server::on_new_conn_cb( DBusServer* server, DBusConnection* conn, void* param )
{
	Server* m = static_cast<Server*>(param);

	Connection nc (conn);

	m->on_new_connection(nc);

	cbus_dbg("incoming connection");
}

Server::Server( const char* address )
{
	Error e;
	_server = dbus_server_listen(address, e);

	if(e)	throw e;

	Monitor::init(_server);

	dbus_server_set_new_connection_function(_server,on_new_conn_cb,this,NULL);
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

void Server::do_dispatch()
{}

}//namespace DBus
