#include "btool_module_database.h"
#include "../bluedebug.h"

namespace Bluetool
{

ModuleDatabase::ModuleDatabase( const std::string& parent, const std::string& conf_root )
:	
	DBus::LocalInterface( BTOOL_MODULEDB_IFACE ),

	DBus::LocalObject( ( parent + BTOOL_MODULEDB_SUBNAME ).c_str() , DBus::Connection::SystemBus() ),

	_conf_root( conf_root )
{
	register_method( ModuleDatabase, ListModules );
	register_method( ModuleDatabase, ReloadModules );
	register_method( ModuleDatabase, SearchClassId );
	register_method( ModuleDatabase, NewInstance );
	register_method( ModuleDatabase, RemoveInstance );
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

			try
			{
				Module* m =
					ModuleLoader::load_module(mname.c_str(), oname() + '/', conf_root );

				_modules.push_back(m);
			}
			catch( Dbg::Error& e )
			{
				blue_dbg("Error loading module '%s': %s", mname.c_str(), e.what());
			}
		}
		closedir( d );
	}
}

ModuleDatabase::~ModuleDatabase()
{
	ModulePList::iterator mi = _modules.begin();
	while( mi != _modules.end() )
	{
		delete *mi;
		++mi;
	}
	InstancePList::iterator ii = _instances.begin();
	while( ii != _instances.end() )
	{
		delete *ii;
		++ii;
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

void ModuleDatabase::ReloadModules ( const DBus::CallMessage& msg )
{
	DBus::ErrorMessage em(msg, BTOOL_ERROR, "Unsupported");
	conn().send(em);
}

void ModuleDatabase::SearchClassId ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		u16 cid = ri.get_uint16();

		Module* m = NULL;

		ModulePList::iterator i = _modules.begin();
		while( i != _modules.end() )
		{
			if( (*i)->provides_service(cid) )
			{
				m = (*i);
				break;
			}
			++i;
		}

		if(!m)
			throw Dbg::Error("Not found");

		const char* mname = m->oname().c_str();

		DBus::ReturnMessage reply(msg);

		reply.append( DBUS_TYPE_STRING, &(mname), DBUS_TYPE_INVALID);
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em ( msg, BTOOL_ERROR, e.what() );
		conn().send(em);
	}
}

void ModuleDatabase::NewInstance ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* mname = ri.get_string();

		Module* m = NULL;

		ModulePList::iterator i = _modules.begin();
		while( i != _modules.end() )
		{
			if( (*i)->oname() == mname )
			{
				m = (*i);
				break;
			}
			++i;
		}

		if(!m)
			throw Dbg::Error("Not found");

		Instance* ic = ModuleLoader::instantiate(m, oname()+'/', _conf_root);

		_instances.push_back(ic);

		const char* ipath = ic->oname().c_str();

		DBus::ReturnMessage reply(msg);

		reply.append( DBUS_TYPE_STRING, &(ipath), DBUS_TYPE_INVALID );
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em ( msg, BTOOL_ERROR, e.what() );
		conn().send(em );
	}
}

void ModuleDatabase::RemoveInstance ( const DBus::CallMessage& msg )
{
	try
	{
		DBus::MessageIter ri = msg.r_iter();

		const char* isname = ri.get_string();

		Instance* is = NULL;

		InstancePList::iterator i = _instances.begin();
		while( i != _instances.end() )
		{
			if( (*i)->oname() == isname )
			{
				is = (*i);
				delete is;
				_instances.erase(i);
				break;
			}
			++i;
		}

		if(!is)
			throw Dbg::Error("Not found");

		DBus::ReturnMessage reply(msg);
		conn().send(reply);
	}
	catch( Dbg::Error& e )
	{
		DBus::ErrorMessage em ( msg, BTOOL_ERROR, e.what() );
		conn().send(em);
	}
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
