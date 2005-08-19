#ifndef __BTOOL_PAN_SERVICE_H
#define __BTOOL_PAN_SERVICE_H

#include <sys/types.h> //for pid_t

#include "../../../libbluetool/bdaddr.h"
#include "../../../cbus/cbus.h"
#include "../../btool_names.h"
#include "../btool_service.h"

#define PAN_SVC_NAME "pan"
#define PAN_CMD "pand"

#define BTOOL_PAN_ERROR	(BTOOL_SVC_ROOT_NAME PAN_SVC_NAME ".error")

class PanService_i : public DBus::LocalInterface
{
public:
	virtual void ListConnections	( const DBus::CallMessage& ) = 0;
	virtual void KillConnection	( const DBus::CallMessage& ) = 0;

protected:
	PanService_i();
};

class PanService : public PanService_i, public BluetoolService
{
public:
	PanService( const std::string& dbus_root, const std::string& conf_root, const BdAddr& addr );

	~PanService();

	void ListConnections	( const DBus::CallMessage& );
	void KillConnection	( const DBus::CallMessage& );

private:
	bool start_service();
	bool stop_service();

private:
	BdAddr	_addr;
	pid_t	_pid;
};

#endif//__BTOOL_PAN_SERVICE_H
