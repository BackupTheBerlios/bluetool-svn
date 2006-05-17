/*
 *
 *  C-Bus - C++ bindings for DBus
 *
 *  Copyright (C) 2005-2006  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <cbus/cbusdebug.h>
#include <cbus/cbusserver.h>

using namespace DBus;

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
