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

#include <cbus/cbusinterface.h>

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

void LocalInterface::emit_signal( SignalMessage& sig )
{
	sig.interface( iname().c_str() );
	remit_signal(sig);
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
