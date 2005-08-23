#ifndef __DBUS_ECHO_H
#define __DBUS_ECHO_H

#include "../../common/eventloop.h"
#include "../cbus.h"

class Echo : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	void echo( const DBus::CallMessage& );
public:
	Echo();
};

#endif//__DBUS_ECHO_H
