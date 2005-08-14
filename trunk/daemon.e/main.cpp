#include <iostream>

#include "../common/eventloop.h"
#include "btool_root_service.h"

int main()
{
	try
	{
		Bluetool::RootService btool_service;

		EventLoop main_loop;

		main_loop.enter();
	}
	catch( std::exception& e )
	{
		std::cout
		<< "Bluetool Daemon terminated (uncaught exception: "
		<< e.what() << " )" << std::endl;
	}
	return 0;
}
