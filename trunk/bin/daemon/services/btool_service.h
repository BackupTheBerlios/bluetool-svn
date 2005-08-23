#ifndef __BTOOL_SERVICE_H
#define __BTOOL_SERVICE_H

#include <string>

#include "../../common/configfile.h"
#include "../../cbus/cbus.h"
#include "../btool_names.h"

namespace Bluetool
{

class Service : public DBus::LocalInterface, public DBus::LocalObject
{
public:
	Service( const std::string& name, const std::string& dbus_root, const std::string& conf_root );

	virtual ~Service();

	bool started();

	/*	methods inherited by all services
	*/
	void Start( const DBus::CallMessage& );
	void Stop( const DBus::CallMessage& );

	void GetOption( const DBus::CallMessage& );
	void SetOption( const DBus::CallMessage& );

private:
	/*	signals emitted by all services
	*/
	void ServiceStarted();
	void ServiceStopped();

private:
	/*	hooks for the various services
	*/
//	virtual bool start_service() = 0;
//	virtual bool stop_service() = 0;

protected:
	ConfigFile settings;

private:
	struct Private;
	Private* pvt;
};

}//namespace Bluetool

#endif//__BTOOL_SERVICE_H
