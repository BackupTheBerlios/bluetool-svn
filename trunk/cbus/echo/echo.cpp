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
	const char* sux ="SUX!";
	reply.append(DBUS_TYPE_STRING, &sux, DBUS_TYPE_INVALID);
	reply.sender(msg.destination());

	std::cout << "reply signature " << reply.signature() << std::endl;
	std::cout << "reply destination " << reply.destination() << std::endl;
	std::cout << "reply serial " << reply.serial() << std::endl;
	std::cout << "reply sender " << reply.sender() << std::endl;

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
