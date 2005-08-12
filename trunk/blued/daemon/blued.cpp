#include <iostream>

#include "../../common/eventloop.h"
#include "../hci/hci_manager.h"

int main()
{
	try
	{
		HciService hci_service;

		EventLoop main_loop;

		main_loop.enter();
	}
	catch( std::exception& e )
	{
		std::cout << "Bluetool Daemon terminated (uncaught exception: " << e.what() << " )" << std::endl;
	}
	return 0;
}
