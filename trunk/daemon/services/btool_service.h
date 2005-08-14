#ifndef __BTOOL_SERVICE_H
#define __BTOOL_SERVICE_H

#include <string>

#include "../../common/configfile.h"
#include "../../cbus/cbus.h"
#include "../btool_names.h"

class BluetoolService : public DBus::LocalInterface, public DBus::LocalObject
{
protected:
	BluetoolService( const std::string& dbus_root, const std::string& conf_root, const std::string& name );

public:
	virtual ~BluetoolService();

	inline bool started();

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
	virtual bool start_service() = 0;
	virtual bool stop_service() = 0;

protected:
	ConfigFile settings;

private:
	bool _started;
};

bool BluetoolService::started()
{
	return _started;
}

#endif//__BTOOL_SERVICE_H
