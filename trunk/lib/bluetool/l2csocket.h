#ifndef __L2CAP_SOCKET_H
#define __L2CAP_SOCKET_H

#include <common/types.h>
#include <common/socket.h>

class L2CapSocket;

#include "bdaddr.h"

/*	Definitions
 */

class L2CapSocket : public Socket
{
public:

	L2CapSocket();

	bool bind( BdAddr& ba );

	bool connect( BdAddr& to, u32 flags );
	
private:

	BdAddr	_from;
	BdAddr	_to;
};

#endif//__L2CAP_SOCKET_H
