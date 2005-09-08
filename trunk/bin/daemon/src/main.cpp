#include <iostream>

#include "signal.h"

#include <common/eventloop.h>
#include "btool_root_service.h"
#include "services/btool_module_loader.h"

EventLoop* main_loop;

void niam( int sig )
{
	main_loop->leave();
}

int main()
{
	signal(SIGTERM, niam);
	signal(SIGINT, niam);

	try
	{
		EventLoop ml;
		main_loop = &ml;

		Bluetool::ModuleLoader::init();
		{
			Bluetool::RootService btool_service;
			Bluetool::DeviceManager	device_manager;

			main_loop->enter();
		}
		Bluetool::ModuleLoader::finalize();
	}
	catch( std::exception& e )
	{
		std::cout
		<< "Bluetool Daemon terminated (uncaught exception: "
		<< e.what() << " )" << std::endl;
	}
	return 0;
}
