#include <bluetooth/bluetooth.h>
#include <bluetooth/bnep.h>

#include "pan_service.h"

#define PAN_SERVICE_NAME	"pan"

PanService_i::PanService_i()
:	DBus::LocalInterface(BTOOL_SVC_ROOT_NAME PAN_SERVICE_NAME)
{
	register_method( PanService_i, ListConnections );
	register_method( PanService_i, KillConnection );
}

PanService::PanService
(
	const std::string& dbus_root,
	const std::string& conf_root,
	const BdAddr& addr
)
:	BluetoolService( dbus_root, conf_root, PAN_SERVICE_NAME ),
	_addr(addr)
{
}

PanService::~PanService()
{
}

bool PanService::start_service()
{
	return true;
}

bool PanService::stop_service()
{
	return true;
}

void PanService::ListConnections( const DBus::CallMessage& )
{
}

void PanService::KillConnection( const DBus::CallMessage& )
{
}
