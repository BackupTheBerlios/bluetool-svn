#include <iostream>
#include "sdp_database.h"

SdpDatabase_i::SdpDatabase_i()
:	DBus::LocalInterface(BTOOL_SERVICE_ROOT"database")
{
	register_method( SdpDatabase_i, AddRecord );
	register_method( SdpDatabase_i, RemoveRecord );
}

SdpDatabase::SdpDatabase()
:	BluetoolService("database")
{
}

void SdpDatabase::AddRecord( const DBus::CallMessage& )
{
	std::cout << "SdpDatabase::AddRecord" << std::endl;
}

void SdpDatabase::RemoveRecord( const DBus::CallMessage& )
{
	std::cout << "SdpDatabase::RemoveRecord" << std::endl;
}

int main()
{
	try
	{
		SdpDatabase db;

		EventLoop mainloop;
		mainloop.enter();
	}
	catch( std::exception& e )
	{
		std::cout << "SDP Database service terminated: " << e.what() << std::endl;
	}
}
