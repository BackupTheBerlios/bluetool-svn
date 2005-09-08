#ifndef __BTOOL_INSTANCE_H
#define __BTOOL_INSTANCE_H

#include <common/error.h>
#include <common/refptr.h>
#include <cbus/cbus.h>
#include "../btool_names.h"
#include "btool_module.h"

namespace Bluetool
{

class Instance;

typedef std::list<Instance*> InstancePList;

class Instance : public DBus::LocalInterface, public DBus::LocalObject
{
public:

	Instance( const Module* mod, const std::string& dbus_root, const std::string& conf_root );

	virtual ~Instance();

	void GetOption		( const DBus::CallMessage& );
	void SetOption		( const DBus::CallMessage& );

	void Action		( const DBus::CallMessage& );

private:
	struct Private;

	RefPtr<Private> pvt;
};

}//namespace Bluetool

#endif//__BTOOL_INSTANCE_H
