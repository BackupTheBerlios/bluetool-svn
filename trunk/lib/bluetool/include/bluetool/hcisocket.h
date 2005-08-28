#ifndef __HCI_SOCKET_H
#define __HCI_SOCKET_H

#include <common/socket.h>
#include <common/types.h>

namespace Hci
{
	class Socket;
}

#include "hcievent.h"
#include "hcifilter.h"
#include "hciexception.h"

namespace Hci
{

class Socket : public ::Socket
{
public:

	Socket();

	bool bind( int id );

	bool set_filter( const Filter& flt );

	bool get_filter( Filter& flt );

private:

//	bool send_cmd( u8 ogf, u8 ocf, void* cmd, int clen );
};

}//namespace Hci

#endif//__HCI_SOCKET_H
