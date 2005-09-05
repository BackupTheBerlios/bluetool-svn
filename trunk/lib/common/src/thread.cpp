#include <common/thread.h>

Thread::Thread()
:	_r (false)
{}

Thread::~Thread()
{}

void* Thread::_thread_fun( void* arg )
{
	Thread* obj = static_cast<Thread*>(arg);

	obj->_r = true;

	obj->run();

	obj->_r = false;

	obj->on_end();

	return NULL;
}

void Thread::start()
{
	pthread_create( & _t, NULL, &_thread_fun, this );
}
