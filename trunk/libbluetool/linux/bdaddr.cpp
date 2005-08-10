#include "../bdaddr.h"

#include <bluetooth/bluetooth.h>

const std::string BdAddr::to_string() const
{
	char straddr[18] = {0};
	ba2str((bdaddr_t*)&(_baddr), straddr);

	return std::string(straddr);
}
