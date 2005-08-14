#ifndef __BTOOL_PAN_SERVICE_H
#define __BTOOL_PAN_SERVICE_H

#include "../../../libbluetool/bdaddr.h"
#include "../../../cbus/cbus.h"
#include "../../btool_names.h"
#include "../btool_service.h"

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
};

#endif//__BTOOL_PAN_SERVICE_H
