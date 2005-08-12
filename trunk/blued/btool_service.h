#ifndef __BTOOL_SERVICE_H
#define __BTOOL_SERVICE_H

/*	DBUS interface to a generic bluetooth service.
	to create your own service, inherit from this class
	and create the appropriate .service file
	to automatically start your service when requested
	by the Bluetooth manager
*/

#include <string>

#include "../cbus/cbus.h"
#include "btool_common.h"

class BluetoolService : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	BluetoolService( const std::string name );

	~BluetoolService();

	/*	methods inherited by all services
	*/
	void Stop( const DBus::CallMessage& );

private:
	/*	signals inherited by all services
	*/
	void ServiceStarted();
	void ServiceStopped();
};

#endif//__BTOOL_SERVICE_H
