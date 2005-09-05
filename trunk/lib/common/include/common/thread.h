#ifndef __SHACK_THREADS_H
#define __SHACK_THREADS_H

#include <pthread.h>

class Thread
{
public:

	Thread();

	virtual ~Thread();

	void start();

	inline bool running();

protected:

	virtual void run() = 0;

	virtual void on_end() = 0;

private:

	static void* _thread_fun( void* arg );

private:

	pthread_t _t;
	bool _r;
};

bool Thread::running()
{	
	return _r;
}

#endif//__SHACK_THREADS_H
