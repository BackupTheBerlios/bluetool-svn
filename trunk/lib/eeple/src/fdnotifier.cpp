#include <eeple/refptr_impl.h>
#include <eeple/debug.h>
#include <algorithm>
#include <eeple/fdnotifier.h>

extern FdNotifierPList g_fdnotifier_plist;

struct FdNotifier::Private
{
	int	fd;
	int	flags;
	int	state;
	void*	data;

	~Private();
};

FdNotifier::Private::~Private()
{
	//_dbg("destroyed  fdnotifier");
}

FdNotifier::FdNotifier( int fd, int flags )
:	pvt(new Private)
{
	pvt->fd = fd;
	pvt->flags = flags;
	pvt->state = 0;
	pvt->data = NULL;

	g_fdnotifier_plist.push_back(this);
}

FdNotifier::~FdNotifier()
{
	FdNotifierPList::iterator i = g_fdnotifier_plist.begin();

	while( i != g_fdnotifier_plist.end() )
	{
		if( *i == this )
		{
			FdNotifierPList::iterator n = i;
			n++;
			g_fdnotifier_plist.erase(i);
			i = n;
		}
		else
		{
			++i;
		}
	}
}

int FdNotifier::fd() const
{
	return pvt->fd;
}

void FdNotifier::fd( int f )
{
	pvt->fd = f;
}

int FdNotifier::flags() const
{
	return pvt->flags;
}

void FdNotifier::flags( int f )
{
	pvt->flags = f;
}

int FdNotifier::state() const
{
	return pvt->state;
}

void FdNotifier::state( int s )
{
	pvt->state = s;
}

void* FdNotifier::data() const
{
	return pvt->data;
}

void FdNotifier::data( void* data )
{
	pvt->data = data;
}

/*
const FdNotifier& FdNotifier::operator = ( const FdNotifier& fn )
{
	if( &fn != this )
	{
		pvt = fn.pvt;
		//ref();
	}
	return (*this);
}
*/
