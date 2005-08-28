#include <cbus/cbusservice.h>

namespace DBus
{
#if 0
LocalService::LocalService( Connection& c, const char* name )
:	_conn (c), _name(name)
{
	c.request_name(name, 0); //TODO: the specification has an additional 'flags' parameter, undocumented
}

LocalService::~LocalService()
{
	//name gets freed automatically when '_conn' dies
}

LocalService LocalService::get( Connection& c, const char* name )
{
	if( c.has_owner(name) )
		return LocalService(c, name);
	else
		throw Error("no such service"); //todo: check if dbus has such a named error
}
#endif

}//namespace DBus
