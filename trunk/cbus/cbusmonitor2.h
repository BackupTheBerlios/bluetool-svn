#ifndef __CBUS_MONITOR_2_H
#define __CBUS_MONITOR_2_H

class CBusMonitor
{

public:

	bool handle_signal( DBusMessage* );

	bool handle_method_call( DBusMessage* );

	//bool handle_error();

protected:

	add/rem/toggle watch|timeout;

private:

	void on_fd_readable( int fd );	//virtuals (not pure) (because a connection will have to do stg more (dispatching))

	void on_fd_writable( int fd );
	//both call dbus_watch_handle

	//todo: use libsig to bind them to FdNotifier classes (not there yet)

	void on_object_destroy(); 	//connection only, the destructor of DBus::Object calls it
};

#endif//__CBUS_MONITOR_2_H
