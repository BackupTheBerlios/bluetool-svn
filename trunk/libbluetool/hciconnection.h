#ifndef __HCI_CONNECTION_H
#define __HCI_CONNECTION_H

#include "../common/types.h"

#include <list>

namespace Hci
{
	class Connection;
}

#include "hcievent.h"
#include "hcidevice.h"
#include "hcifilter.h"

namespace Hci
{

class Connection
{
public:

	Connection( LocalDevice& from, RemoteDevice& to );

	virtual ~Connection();

	LocalDevice& from()
	{
		return _from;
	}

	RemoteDevice& to()
	{
		return _to;
	}

	u16 handle()
	{
		return _handle;
	}

	bool set_filter( const Filter& flt );

public:
	
	Event	on_event;

public:
	/*	commands
	*/

private:
	LocalDevice&	_from;
	RemoteDevice&	_to;
	Socket&		_dd;
	u16		_handle;
};

typedef std::list<Connection> ConnectionList;

}//namespace Hci

#endif//__HCI_CONNECTION_H
