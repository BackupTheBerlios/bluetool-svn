#include "btool_service_database.h"

namespace Bluetool
{

ServiceDatabase::ServiceDatabase( const std::string& parent, const std::string& conf_root )
:	
	DBus::LocalInterface( BTOOL_SERVICEDB_IFACE ),

	DBus::LocalObject( ( parent + BTOOL_SERVICEDB_SUBNAME ).c_str() , DBus::Connection::SystemBus() ),

	_conf_root( conf_root )
{
	register_method( ServiceDatabase, ListServices );
	register_method( ServiceDatabase, LoadService );
	register_method( ServiceDatabase, UnloadService );
}

ServiceDatabase::~ServiceDatabase()
{
	ServicePList::iterator i = _services.begin();
	while( i != _services.end() )
	{
		delete *i;
		++i;
	}
}

/*	methods
*/
void ServiceDatabase::ListServices ( const DBus::CallMessage& )
{
}

void ServiceDatabase::LoadService ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* svc_name = ri.get_string();

		Service* svc = ServiceLoader::load_service(svc_name, oname() + '/', _conf_root);

		_services.push_back(svc);

		const char* instance_path = svc->oname().c_str();

		DBus::ReturnMessage reply(msg);
		reply.append(	DBUS_TYPE_STRING, &(instance_path),
				DBUS_TYPE_INVALID
		);
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em (msg, BTOOL_ERROR, e.what());
		conn().send(em);
	}
}

void ServiceDatabase::UnloadService ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* path_name = ri.get_string();

		bool found = false;

		ServicePList::iterator i = _services.begin();

		while( i != _services.end() )
		{
			if( (*i)->oname() == path_name )
			{
				delete *i;
				_services.erase(i);
				found = true;
				break;
			}
			++i;
		}

		if(!found)
		{
			DBus::ErrorMessage reply (msg, BTOOL_ERROR, "path not found");
			conn().send(reply);
		}
		else
		{
			DBus::ReturnMessage reply(msg);
			conn().send(reply);
		}

	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em (msg, BTOOL_ERROR, e.what());
		conn().send(em);
	}
}

}//namespace Bluetool
