#ifndef __CBUS_MESSAGE_H
#define __CBUS_MESSAGE_H

#include <dbus/dbus.h>
#include <string>
#include <map>

namespace DBus
{
class Message;

class ErrorMessage;
class SignalMessage;
class CallMessage;
class ReturnMessage;

class MessageIter;
}

#include "cbuserror.h"
//#include "cbusconnection.h"

namespace DBus
{
class Connection;
/*
*/

class MessageIter
{
public:
	//MessageIter( Message& );

	inline int type();

	inline bool at_end();

	inline bool has_next();

	inline MessageIter operator ++();

	inline MessageIter& operator ++(int);

	inline bool append_string( const char* string );

	inline const char* get_string();

	inline bool append_bool( bool b );

	inline bool get_bool();

	inline MessageIter recurse();

private:
	DBusMessageIter _iter;

friend class Message;
};

/*
*/
int MessageIter::type()
{
	return dbus_message_iter_get_arg_type(&_iter);
}

bool MessageIter::at_end()
{
	return type() == DBUS_TYPE_INVALID;
}
bool MessageIter::has_next()
{
	return dbus_message_iter_has_next(&_iter);
}
MessageIter MessageIter::operator ++()
{
	MessageIter copy(*this);
	(*this)++;
	return copy;
}
MessageIter& MessageIter::operator ++(int)
{
	dbus_message_iter_next(&_iter);
	return (*this);
}

bool MessageIter::append_string( const char* string )
{
	return dbus_message_iter_append_basic(&_iter, DBUS_TYPE_STRING, &string);
}

const char* MessageIter::get_string()
{
 	char* ret;
	dbus_message_iter_get_basic(&_iter, &ret);
 	return ret;
}

bool MessageIter::append_bool( bool b )
{
	return dbus_message_iter_append_basic(&_iter, DBUS_TYPE_BOOLEAN, &b);
}

bool MessageIter::get_bool()	
{	
 	bool ret;
	dbus_message_iter_get_basic(&_iter, &ret);
 	return ret;
}

MessageIter MessageIter::recurse() 
{
	MessageIter iter;
	dbus_message_iter_recurse(&_iter, &iter._iter);
	return iter;
}

/*
*/

class Message
{
public:
	Message();

	Message( const Message& m );

	Message( DBusMessage* );

	~Message();

	inline int type() const;

	inline int serial() const;

	inline bool serial( int s );

	inline const char* sender() const;

	inline bool sender( const char* s );

	inline const char* destination() const;

	inline bool destination( const char* s );

	inline bool is_error();

	inline MessageIter r_iter() const;

	inline MessageIter w_iter();

	bool read( const char* format, int first_type, ... ) const;

	bool write( const char* format, int first_type, ... );

protected:

	inline void ref();

	inline void unref();

protected:
	DBusMessage *_message;

/*	classes who need to read `_message` directly
*/
friend	class ErrorMessage;
friend	class ReturnMessage;
friend	class MessageIter;
friend	class Error;
friend	class Connection;
};

/*
*/

int Message::type() const
{
	return dbus_message_get_type(_message);
}
int Message::serial() const
{
	return dbus_message_get_reply_serial(_message);
}
bool Message::serial( int s )
{
	return dbus_message_set_reply_serial(_message,s);
}
const char* Message::sender() const
{
	return dbus_message_get_sender(_message);
}
bool Message::sender( const char* s )
{
	return dbus_message_set_sender(_message, s);
}
const char* Message::destination() const
{
	return dbus_message_get_destination(_message);
}
bool Message::destination( const char* s )
{
	return dbus_message_set_destination(_message, s);
}

bool Message::is_error()
{
	return type() == DBUS_MESSAGE_TYPE_ERROR;
}

MessageIter Message::w_iter()
{
	MessageIter iter;
	dbus_message_iter_init_append(_message, &iter._iter);
	return iter;
}

MessageIter Message::r_iter() const
{
	MessageIter iter;
	dbus_message_iter_init(_message, &iter._iter);
	return iter;
}

void Message::ref()
{
	dbus_message_ref(_message);
}
void Message::unref()
{
	dbus_message_unref(_message);
}

/*
*/

class ErrorMessage : public Message
{
public:
	ErrorMessage( Message&, const char* name, const char* message );

	inline const char* name() const;

	inline bool name( const char* n );

	bool operator == ( const ErrorMessage& ) const;
};

/*
*/

const char* ErrorMessage::name() const
{
	return dbus_message_get_error_name(_message);
}
bool ErrorMessage::name( const char* n )
{
	return dbus_message_set_error_name(_message, n);
}

/*
*/

class SignalMessage : public Message
{
public:
	SignalMessage( const char* path, const char* interface, const char* name );

	inline const char* interface() const;

	inline bool interface( const char* i );

	inline const char* member() const;

	inline bool member( const char* m );

	inline const char* path() const;

	inline char** path_split() const;

	inline bool path( const char* p );

	bool operator == ( const SignalMessage& ) const;
};

/*
*/

const char* SignalMessage::interface() const
{
	return dbus_message_get_interface(_message);
}
bool SignalMessage::interface( const char* i )
{
	return dbus_message_set_interface(_message, i);
}
const char* SignalMessage::member() const
{
	return dbus_message_get_member(_message);
}
bool SignalMessage::member( const char* m )
{
	return dbus_message_set_member(_message, m);
}
const char* SignalMessage::path() const
{
	return dbus_message_get_path(_message);
}
char** SignalMessage::path_split() const
{
	char** p;
	dbus_message_get_path_decomposed(_message, &p);	//todo: return as a std::vector ?
	return p;
}
bool SignalMessage::path( const char* p )
{
	return dbus_message_set_path(_message, p);
}

/*
*/

class CallMessage : public Message
{
public:
	CallMessage( const char* dest, const char* path, const char* iface, const char* method );

	inline const char* interface() const;

	inline bool interface( const char* i );

	inline const char* member() const;

	inline bool member( const char* m );

	inline const char* path() const;

	inline char** path_split() const;

	inline bool path( const char* p );

	inline const char* signature() const;

	bool operator == ( const CallMessage& ) const;
};

/*
*/

const char* CallMessage::interface() const
{
	return dbus_message_get_interface(_message);
}
bool CallMessage::interface( const char* i )
{
	return dbus_message_set_interface(_message, i);
}
const char* CallMessage::member() const
{
	return dbus_message_get_member(_message);
}
bool CallMessage::member( const char* m )
{
	return dbus_message_set_member(_message, m);
}
const char* CallMessage::path() const
{
	return dbus_message_get_path(_message);
}
char** CallMessage::path_split() const
{
	char** p;
	dbus_message_get_path_decomposed(_message, &p);	//todo: return as a std::vector ?
	return p;
}
bool CallMessage::path( const char* p )
{
	return dbus_message_set_path(_message, p);
}

const char* CallMessage::signature() const
{
	return dbus_message_get_signature(_message);
}

/*
*/

class ReturnMessage : public Message
{
public:
	ReturnMessage( const CallMessage& callee );

	inline const char* signature() const;
};

/*
*/

const char* ReturnMessage::signature() const
{
	return dbus_message_get_signature(_message);
}

}//namespace DBus

#endif//__CBUS_MESSAGE_H
