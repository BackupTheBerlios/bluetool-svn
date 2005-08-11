#include "../bdaddr.h"

#include <bluetooth/bluetooth.h>

BdAddr::BdAddr()
{
	memset(_baddr,0,6);
}

BdAddr::BdAddr( const u8* data )
{
	memcpy(_baddr,data,6);
}

const std::string BdAddr::to_string() const
{
	char straddr[18] = {0};
	ba2str((bdaddr_t*)&(_baddr), straddr);

	return std::string(straddr);
}
