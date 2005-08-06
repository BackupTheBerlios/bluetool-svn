#include "../l2csocket.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sdp.h>

L2CapSocket::L2CapSocket()
:	Socket( PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP )
{}

bool L2CapSocket::bind( BdAddr& ba )
{
	sockaddr_l2 saddr;
	saddr.l2_family = AF_BLUETOOTH;
	saddr.l2_psm = 0;

	return Socket::bind( (sockaddr*)&saddr, sizeof(saddr) );
}

bool L2CapSocket::connect( BdAddr& to, u32 flags )
{
	sockaddr_l2 saddr;
	saddr.l2_family = AF_BLUETOOTH;
	saddr.l2_psm = htobs(SDP_PSM);
	memcpy(&saddr.l2_bdaddr, to.addr(), sizeof(saddr.l2_bdaddr));

	return Socket::connect( (sockaddr*)&saddr, sizeof(saddr) );
	//todo: retry if busy
}
