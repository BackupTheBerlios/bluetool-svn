#ifndef __ECHO_SERVER_H
#define __ECHO_SERVER_H

#include <cbus/cbus.h>

class EchoInterface : 
	public DBus::LocalInterface
{
public:

	EchoInterface() : DBus::LocalInterface("org.test.cbus.EchoServer")
	{
		register_method(EchoInterface, Echo);
	}

	virtual void Echo( const DBus::CallMessage& ) = 0;
};

#endif//__ECHO_SERVER_H
