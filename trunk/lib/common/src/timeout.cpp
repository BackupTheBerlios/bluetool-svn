#include <sys/time.h>
#include <algorithm>
#include <common/timeout.h>

extern TimeoutRList g_timeout_rlist;

Timeout* Timeout::create( int interval )
{
	Timeout* nt = new Timeout(interval);

	RefPtr<Timeout> rt (nt);

	g_timeout_rlist.push_back(rt);

	return nt;
}

void Timeout::destroy( Timeout* t )
{
	TimeoutRList::iterator i = g_timeout_rlist.begin();

	while( i != g_timeout_rlist.end() )
	{
		if( i->get() == t )
		{
			TimeoutRList::iterator n = i;
			n++;
			g_timeout_rlist.erase(i);
			i = n;
		}
		else
		{
			++i;
		}
	}
}

struct Timeout::Private
{
	bool	on;
	double	started;
	int	interval;
	double	timeslice;
	void*	data;
};

Timeout::Timeout( int interval )
:	pvt ( new Private )
{
	pvt->interval = interval;
	pvt->timeslice = interval;
	pvt->data = NULL;
	pvt->on = interval ? true : false;

	timeval started;
	gettimeofday(&started, NULL);

	pvt->started = started.tv_sec*1000 + started.tv_usec/1000.0;
}

Timeout::~Timeout()
{}

int Timeout::interval() const
{
	return pvt->interval;
}

//int Timeout::timeslice() const
//{
//	return _timeslice;
//}

void Timeout::interval( int d )
{
	pvt->interval = d;
}

//void Timeout::timeslice( int d )
//{
//	_timeslice = d;
//}

void Timeout::data( void* data )
{
	pvt->data = data;
}

void* Timeout::data() const
{
	return pvt->data;
}

void Timeout::start()
{
	pvt->on = true;
}

void Timeout::stop()
{
	pvt->on = false;
}

bool Timeout::on()
{
	return pvt->on;
}

bool Timeout::update()
{	
	if(!on()) return false;

	timeval now;
	gettimeofday(&now, NULL);

	double now_millis = now.tv_sec*1000 + now.tv_usec/1000.0;

	if(now_millis >= pvt->started + pvt->interval)
	{
		pvt->started = now_millis;
		return true;
	}
	return false;
}

/*
const Timeout& Timeout::operator = ( const Timeout& t )
{
	if( this != &t )
	{
		pvt = t.pvt;
		ref();	
	}
	return (*this);
}*/
