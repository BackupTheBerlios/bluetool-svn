#ifndef __SHACK_MUTEX_H
#define __SHACK_MUTEX_H

#include <pthread.h>

class Mutex()
{
public:
	Mutex()
	{
		pthread_mutex_init( & _m, NULL );
	}

	~Mutex()
	{
		pthread_mutex_destroy( & _m );
	}

	void lock()
	{
		pthread_mutex_lock( & _m );
	}

	void unlock()
	{
		pthread_mutex_unlock( & _m );
	}

private:

	pthread_mutex_t _m;
};

#endif//__SHACK_MUTEX_H
