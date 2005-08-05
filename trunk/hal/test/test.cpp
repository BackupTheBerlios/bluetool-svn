#include <iostream>

#include "../../common/eventloop.h"
#include "../halmanager.h"

int main()
{
	try
	{
		HalManager hal_manager;

		EventLoop main_loop;
		main_loop.enter();
	}
	catch( DBus::Error& e )
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
