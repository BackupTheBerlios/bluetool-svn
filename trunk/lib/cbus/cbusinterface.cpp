#include "cbusdebug.h"

#include "cbusinterface.h"

namespace DBus
{

Interface::Interface( const char* name )
:	_name(name)
{
	this->register_interface(name);
}

Interface::~Interface()
{
	//this->unregister_interface(name);	//not needed
}

bool Interface::invoke_method( const CallMessage& )
{
	return false;
}

bool Interface::dispatch_signal( const SignalMessage& )
{
	return false;
}

void Interface::register_interface( const char* name )
{
	cbus_dbg("registering interface %s",name);

	_interfaces[name] = this;
}

LocalInterface::LocalInterface( const char* name )
:	Interface(name)
{}

bool LocalInterface::invoke_method( const CallMessage& msg )
{
	const char* name = msg.member();

	MethodTable::const_iterator mi = _methods.find(name);
	if( mi != _methods.end() )
	{
		mi->second( msg );
		return true;
	}
	else	return false;
}

RemoteInterface::RemoteInterface( const char* name )
:	Interface(name)
{}

bool RemoteInterface::dispatch_signal( const SignalMessage& msg )
{
	const char* name = msg.member();

	SignalTable::const_iterator si = _signals.find(name);
	if( si != _signals.end() )
	{
		si->second( msg );
		return true;
	}
	else	return false;
}

}//namespace DBus
