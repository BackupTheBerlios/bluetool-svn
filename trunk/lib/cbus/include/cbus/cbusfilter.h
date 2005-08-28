#ifndef __CBUS_FILTER_H
#define __CBUS_FILTER_H

#include <list>
#include <string>
#include <sigc++/sigc++.h>
#include <dbus/dbus.h>

namespace DBus
{
class Filter;

class Message;
}

//#include "cbusconnection.h"
//#include "cbusmessage.h"

namespace  DBus
{
typedef sigc::signal<bool, /*Connection&, */Message&> Filtered;

class Filter
{
public:
//	Filter( const char* rule );

//	inline const char* rule() const;

	Filtered	filtered;
private:

};

typedef std::list<Filter> FilterList;

//const char* Filter::rule() const
//{
//	return _rule.c_str();
//}


}//namespace DBus

#endif//__CBUS_FILTER_H
