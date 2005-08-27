#include <sigc++/sigc++.h>
#include "cbusdebug.h"
#include "cbusmonitor.h"

namespace DBus
{

struct Monitor::Private
{
	static void on_new_conn( DBusServer* server, DBusConnection* conn, void* param )
	{
		//Private* m = static_cast<Private*>(param);
		//Connection c = Connection(conn);
		//m->on_new_connection.emit(c);

		cbus_dbg("incoming connection");
	}

	static dbus_bool_t on_add_watch( DBusWatch* watch, void* data )
	{
		Private* m = static_cast<Private*>(data);
		m->add_watch(watch);
		return true;
	}

	static void on_rem_watch( DBusWatch* watch, void* data )
	{
		Private* m = static_cast<Private*>(data);
		m->rem_watch(watch);
	}

	static void on_toggle_watch( DBusWatch* watch, void* data )
	{
		//Private* m = static_cast<Private*>(data);
		FdNotifier* fn = static_cast<FdNotifier*>(dbus_watch_get_data(watch));

		if(dbus_watch_get_enabled(watch))
		{
			fn->flags(0);
		}
		else
		{
			int flags = dbus_watch_get_flags(watch);
			int pflags = 0;
	
			if( flags & DBUS_WATCH_READABLE )
				pflags |= POLLIN;
			if( flags & DBUS_WATCH_WRITABLE )
				pflags |= POLLOUT;

			fn->flags(pflags);
		}
	}

	static dbus_bool_t on_add_timeout( DBusTimeout* timeout, void* data )
	{
		Private* m = static_cast<Private*>(data);
		m->add_timeout(timeout);
		return true;
	}

	static void on_rem_timeout( DBusTimeout* timeout, void* data )
	{
		Private* m = static_cast<Private*>(data);
		m->rem_timeout(timeout);
	}

	static void on_toggle_timeout( DBusTimeout* timeout, void* data )
	{
		//Private* m = static_cast<Private*>(data);
		Timeout* t = static_cast<Timeout*>(dbus_timeout_get_data(timeout));
		if(dbus_timeout_get_enabled(timeout))
			t->stop();
		else
			t->start();
	}

	void add_watch( DBusWatch* );

	void rem_watch( DBusWatch* );

	void add_timeout( DBusTimeout* );

	void rem_timeout( DBusTimeout* );

	void fd_ready( FdNotifier& );

	//void fd_ready_out( FdNotifier& );

	void timeout_ready( Timeout& );

	union {	
		DBusConnection* connection;
		DBusServer*	server;
	} ptr;

	Monitor* parent;

};//Private

void Monitor::Private::add_watch( DBusWatch* w )
{
	cbus_dbg("adding watch %p",w);

	int wflags = dbus_watch_get_flags(w);
	int flags = 0;

	if( wflags & DBUS_WATCH_READABLE )
		flags |= POLLIN;

	//todo, it's buggy
//	if( wflags & DBUS_WATCH_WRITABLE )
//		flags |= POLLOUT;

	FdNotifier* fn = FdNotifier::create( dbus_watch_get_fd(w), flags );

	fn->data(w);
	fn->can_read.connect( sigc::mem_fun(*this, &Monitor::Private::fd_ready) );
	fn->can_write.connect( sigc::mem_fun(*this, &Monitor::Private::fd_ready) );

	
 	dbus_watch_set_data(w, fn, 0);

	//_fdnotifiers.push_back(fn);
}
void Monitor::Private::rem_watch( DBusWatch* w )
{
	cbus_dbg("removing watch %p",w);

	FdNotifier* fn = static_cast<FdNotifier*>(dbus_watch_get_data(w));

	FdNotifier::destroy(fn);

/*	FdNotifierPList::iterator i = _fdnotifiers.begin();
	while( i != _fdnotifiers.end() )
	{
		if( (*i)->data() == w )
		{
			delete *i;
			_fdnotifiers.erase(i);
			break;
		}
		++i;
	}
*/
}

void Monitor::Private::add_timeout( DBusTimeout* dt )
{
	Timeout *t = Timeout::create();

	t->data(dt);
	t->interval(dbus_timeout_get_interval(dt));
	t->timed_out.connect( sigc::mem_fun(*this, &Monitor::Private::timeout_ready) );
	t->start();

	cbus_dbg("adding timeout %p with interval %d", dt, t->interval());

	dbus_timeout_set_data(dt, t, 0);

//	_timeouts.push_back(t);
}
void Monitor::Private::rem_timeout( DBusTimeout* dt )
{
	cbus_dbg("removing timeout %p", dt);

	Timeout* t = static_cast<Timeout*>(dbus_timeout_get_data(dt));
	t->update();

	Timeout::destroy(t);

/*	TimeoutPList::iterator i = _timeouts.begin();
	while( i != _timeouts.end() )
	{
		if( (*i)->data() == dt )
		{
			(*i)->update();
			delete *i;
			_timeouts.erase(i);
			break;
		}
		++i;
	}
*/
}

void Monitor::do_dispatch()
{
	cbus_dbg("pure virtual do_dispatch() called");
}

void Monitor::Private::fd_ready( FdNotifier& fn )
{
	DBusWatch* dw = static_cast<DBusWatch*>(fn.data());

	/*	to notify the library internals
	*/
	int flags = 0;

	if (fn.state() & POLLIN)
		flags |= DBUS_WATCH_READABLE;
	if (fn.state() & POLLOUT)
		flags |= DBUS_WATCH_WRITABLE;
	if (fn.state() & POLLHUP)
		flags |= DBUS_WATCH_HANGUP;
	if (fn.state() & POLLERR)
		flags |= DBUS_WATCH_ERROR;

	dbus_watch_handle(dw, flags);

	cbus_dbg("watch %p ready (%c%c%c%c)", dw, 	fn.state() & POLLIN  ? 'R' : '_',
							fn.state() & POLLOUT ? 'W' : '_',
							fn.state() & POLLHUP ? 'H' : '_',
							fn.state() & POLLERR ? 'E' : '_');


	parent->do_dispatch();
}

#if 0
void Monitor::Private::fd_ready_out( FdNotifier& fn )
{
	DBusWatch* dw = static_cast<DBusWatch*>(fn.data());

	cbus_dbg("watch %p ready (O)", dw);

	/*	to notify the library internals
	*/
	dbus_watch_handle(dw, DBUS_WATCH_WRITABLE);

	parent->do_dispatch();

	if(parent->_dispatch_pending)
	{
		cbus_dbg("dispatching on %p", ptr.connection);

		while (dbus_connection_dispatch(ptr.connection) == DBUS_DISPATCH_DATA_REMAINS) 
		; //loop
		parent->_dispatch_pending = false;
	}
}
#endif

void Monitor::Private::timeout_ready( Timeout& t )
{
	DBusTimeout* dt = static_cast<DBusTimeout*>(t.data());

	cbus_dbg("timeout %p expired", dt);

	t.interval(dbus_timeout_get_interval(dt));
//	t.timeslice(dbus_timeout_get_interval(dt));

	dbus_timeout_handle(dt);
}

/*
*/

Monitor::Monitor()
:	pvtslot(-1)
{
	//pvt = new Private();
	//pvt->parent = this;
}

void Monitor::init( DBusServer* server )
{
	cbus_dbg("registering stubs for server %p", server);

	dbus_server_allocate_data_slot(&pvtslot);

	Private* pvt = (Private*)malloc(sizeof(Private));
	pvt->parent = this;

	dbus_server_set_data(server, pvtslot, pvt, free);

	dbus_server_set_new_connection_function(server, Private::on_new_conn, pvt, 0);

	dbus_server_set_watch_functions( server,
					 Private::on_add_watch,
					 Private::on_rem_watch,
					 Private::on_toggle_watch,
					 pvt, 0);

	dbus_server_set_timeout_functions( server,
					   Private::on_add_timeout,
					   Private::on_rem_timeout,
					   Private::on_toggle_timeout,
					   pvt, 0);

	pvt->ptr.server = server;
}

void Monitor::init( DBusConnection* connection )
{
	cbus_dbg("registering stubs for connection %p", connection);

	dbus_connection_allocate_data_slot(&pvtslot);

	Private* pvt = (Private*)malloc(sizeof(Private));
	pvt->parent = this;

	dbus_connection_set_data(connection, pvtslot, pvt, dbus_free);

	dbus_connection_set_watch_functions(   connection,
					       Private::on_add_watch,
					       Private::on_rem_watch,
					       Private::on_toggle_watch,
					       pvt, 0);

	dbus_connection_set_timeout_functions( connection,
					       Private::on_add_timeout,
					       Private::on_rem_timeout,
					       Private::on_toggle_timeout,
					       pvt, 0);

	pvt->ptr.connection = connection;
}

Monitor::~Monitor()
{
//	if(noref())	delete pvt;

/*	TimeoutPList::iterator ti = _timeouts.begin();
	while( ti != _timeouts.end() )
	{
		delete *ti;
		++ti;
	}
*/
/*
	FdNotifierPList::iterator fi = _fdnotifiers.begin();
	while( fi != _fdnotifiers.end() )
	{
		delete *fi;
		++fi;
	}
*/
}

}//namespace DBus
