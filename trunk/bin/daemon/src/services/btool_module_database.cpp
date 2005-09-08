#include "btool_module_database.h"

namespace Bluetool
{

ModuleDatabase::ModuleDatabase( const std::string& parent, const std::string& conf_root )
:	
	DBus::LocalInterface( BTOOL_MODULEDB_IFACE ),

	DBus::LocalObject( ( parent + BTOOL_MODULEDB_SUBNAME ).c_str() , DBus::Connection::SystemBus() ),

	_conf_root( conf_root )
{
	register_method( ModuleDatabase, ListModules );
//	register_method( ModuleDatabase, LoadService );
//	register_method( ModuleDatabase, UnloadService );

	DIR* d = opendir("../../extras/modules"); //todo: never hardcode paths

	if( d )
	{
		dirent* entry;
		while( (entry = readdir(d)) != NULL )
		{
			int l = strlen(entry->d_name);

			if ( strcmp(entry->d_name + l - 3, ".py") )
				continue;

			std::string mname (entry->d_name, 0, l-3 );

			Module* m = ModuleLoader::load_module(mname.c_str(), oname() + '/', conf_root );

			if ( m )
				_modules.push_back(m);
		}
		closedir( d );
	}
}

ModuleDatabase::~ModuleDatabase()
{
	ModulePList::iterator i = _modules.begin();
	while( i != _modules.end() )
	{
		delete *i;
		++i;
	}
}

/*	methods
*/
void ModuleDatabase::ListModules ( const DBus::CallMessage& msg )
{
	DBus::ReturnMessage reply (msg);

	DBus::MessageIter wi = reply.w_iter();
	DBus::MessageIter ai = wi.new_array(DBUS_TYPE_STRING);

	ModulePList::iterator i = _modules.begin();
	while( i != _modules.end() )
	{
		const char* p = (*i)->oname().c_str();
		ai.append_string(p);
		++i;
	}

	wi.close_container(ai);
	conn().send(reply);
}

#if 0
void ModuleDatabase::LoadModule ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* svc_name = ri.get_string();

		Service* svc = ServiceLoader::load_service(svc_name, oname() + '/', _conf_root);

		_modules.push_back(svc);

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

void ModuleDatabase::UnloadModule ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* path_name = ri.get_string();

		bool found = false;

		ModulePList::iterator i = _modules.begin();

		while( i != _modules.end() )
		{
			if( (*i)->oname() == path_name )
			{
				delete *i;
				_modules.erase(i);
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
#endif

}//namespace Bluetool
