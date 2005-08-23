#ifndef __CBUS_ERROR_H
#define __CBUS_ERROR_H

//#include <string>
#include <exception>
#include <dbus/dbus.h>

namespace DBus
{
class Error;
class Message;
}

namespace DBus
{

class Error : public std::exception
{
public:
	Error();

	Error( const char* name, const char* message );
	
	explicit Error( DBusError* error );

	Error( Message& );

	~Error() throw();

	const char* what() const throw();

	const char* name()
	{
		return _error.name;
	}

	const char* message()
	{
		return _error.message;
	}

	operator bool()
	{
		return is_set();
	}

	operator DBusError*() //ouch!
	{
		return &_error;
	}

	bool is_set()
	{
		return dbus_error_is_set(&_error);
	}

	void set( const char* name, const char* message );	//todo: parameters MUST be statics

//	void set( const char* name, const char* format, ... )
//	{
//		return dbus_set_error(&_error, name, message);
//	}

private:
	DBusError	_error;
//	std::string	_name;
//	std::string	_message;
};

}//namespace DBus

#endif//__CBUS_ERROR_H
