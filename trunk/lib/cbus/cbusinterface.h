#ifndef __CBUS_INTERFACE_H
#define __CBUS_INTERFACE_H

#include <string>
#include <map>
#include <sigc++/sigc++.h>
#include "../common/refcnt.h"

namespace DBus
{
	class Interface;
	class IfaceTracker;

	typedef std::map<std::string, Interface*>	InterfaceTable;

	class IfaceTracker : public RefCnt 
	{
	protected:

		InterfaceTable	_interfaces;
	};

	class CallMessage;
	class SignalMessage;

	typedef sigc::signal<void, const CallMessage&> MethodCallback;
	typedef sigc::signal<void, const SignalMessage&> SignalCallback;

	typedef std::map<std::string, MethodCallback>	MethodTable;
	typedef std::map<std::string, SignalCallback>	SignalTable;

#	define register_method(interface, method)	\
	LocalInterface::_methods[ #method ].connect( sigc::mem_fun(*this, & interface :: method ) );

#	define connect_signal(interface, signal)	\
	RemoteInterface::_signals[ #signal ].connect( sigc::mem_fun(*this, & interface :: signal ) );

//	this->conn().add_match("member='" #signal "'");

class Interface : protected virtual IfaceTracker
{
public:
	
	Interface( const char* name );
	
	virtual ~Interface();

	inline const std::string& iname() const;

	virtual bool invoke_method( const CallMessage& );

	virtual bool dispatch_signal( const SignalMessage& );

private:

	void register_interface( const char* name );

private:

	std::string _name;
};

}

//#include "cbusobject.h"
#include "cbusmessage.h"

namespace DBus
{

/*
*/

const std::string& Interface::iname() const
{
	return _name;
}

/*
*/

class LocalInterface : public Interface
{
public:
	LocalInterface( const char* name );

	bool invoke_method( const CallMessage& );

protected:

	MethodTable	_methods;
};

/*
*/

class RemoteInterface : public Interface
{
public:

	RemoteInterface( const char* name );

	bool dispatch_signal( const SignalMessage& );

protected:

	SignalTable	_signals;
};

/*
[example use case]

class MyInterface : public DBus::LocalInterface
{
protected:

	virtual bool MyMethod( const CallMessage& ) = 0;
public:
	MyInterface()
	: DBus::LocalInterface("org.whatever.servicecorp.my_interface")
	{
		register_method(MyInterface, MyMethod);
	}
};

class MyObject : public DBus::LocalObject, public MyInterface
{
public:
	MyObject()
	: DBus::LocalObject("/org/whatever/servicecorp/funny_object")
	{}
protected:
	bool MyMethod( const CallMessage& );
};

*/

}

#endif//__CBUS_INTERFACE_H
