#ifndef __BTOOL_MODULE_H
#define __BTOOL_MODULE_H

#include <common/error.h>
#include <common/refptr.h>
#include <cbus/cbus.h>
#include "../btool_names.h"

namespace Bluetool
{

class Module;

typedef std::list<Module*> ModulePList;

class Module : public DBus::LocalInterface, public DBus::LocalObject
{
public:

	Module( const std::string& name, const std::string& dbus_root, const std::string& conf_root );

	~Module();

	const std::string& name() const;

	//void Instance ( const DBus::CallMessage& );

	void Description ( const DBus::CallMessage& );

private:

	struct Private;

	RefPtr<Private> pvt;

friend class Service;
};

}//namespace Bluetool

#endif//__BTOOL_MODULE_H
