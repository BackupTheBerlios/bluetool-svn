#ifndef __CBUS_TIMEOUT_H
#define __CBUS_TIMEOUT_H

#include <list>
#include <sigc++/sigc++.h>
#include "refptr.h"

class Timeout;

typedef sigc::signal<void, Timeout&> TimedOut;

class Timeout
{
public:
	static Timeout* create( int interval = 0 );

	static void destroy( Timeout* t );

	~Timeout();

private:
	Timeout( int interval );

public:
//	const Timeout& operator = ( const Timeout& );

	int interval() const;
//	inline double timeslice() const;

	void interval( int );
//	inline double timeslice( int );

	void data( void* );
	void* data() const;

	void start();
	void stop();
	bool on();

	bool update();

	TimedOut timed_out;

private:
	struct Private;
	RefPtr<Private> pvt;
};

typedef std::list<Timeout> TimeoutList;
typedef std::list< RefPtr<Timeout> > TimeoutRList;

#endif//__CBUS_TIMEOUT_H
