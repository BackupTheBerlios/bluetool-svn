#include <iostream>

#include "echo.h"

Echo::Echo()
:	DBus::LocalInterface( "org.emptyspace.echo" ),
	DBus::LocalObject( "/org/emptyspace/echo", DBus::Connection::SystemBus() )
{
	conn().request_name("org.emptyspace");

	register_method( Echo, echo );
}

void Echo::echo( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);

	conn().send(reply);
}

int main()
{
	try
	{
		Echo echoservice;

		EventLoop mainloop;
		mainloop.enter();
	}
	catch( std::exception& e )
	{
		std::cout << "program terminated: " << e.what() << std::endl;
	}
}
