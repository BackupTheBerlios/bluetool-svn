#include <bluetool/hcisocket.h>
#include "hcifilter_p.h"

#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

namespace Hci
{

Socket::Socket()
:	::Socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
{}

bool Socket::bind( int id )
{
	sockaddr_hci _saddr;

	_saddr.hci_family = AF_BLUETOOTH;
	_saddr.hci_dev = id;

	return ::Socket::bind((sockaddr*)&_saddr, sizeof(_saddr));
}

bool Socket::get_filter( Filter& flt )
{
	socklen_t len = sizeof(flt.pvt->filter);
	return ::getsockopt(this->handle(), SOL_HCI, HCI_FILTER, &(flt.pvt->filter), &len) >= 0;
}

bool Socket::set_filter( const Filter& flt )
{
	return ::setsockopt(this->handle(), SOL_HCI, HCI_FILTER, &(flt.pvt->filter), sizeof(flt.pvt->filter) ) >= 0;
}

}//namespace Hci
