#ifndef __CBUS_TIMEOUT_H
#define __CBUS_TIMEOUT_H

#include <list>
#include <sigc++/sigc++.h>
#include "refcnt.h"

class Timeout;

typedef sigc::signal<void, Timeout&> TimedOut;

class Timeout : public RefCnt
{
public:
	Timeout( int interval = 0 );

	~Timeout();

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

	void update();

	TimedOut timed_out;

private:
	struct Private;
	Private* pvt;
};

typedef std::list<Timeout> TimeoutList;
typedef std::list<Timeout*> TimeoutPList;

#endif//__CBUS_TIMEOUT_H
