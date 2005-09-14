#include <common/refptr_impl.h>
#include <common/debug.h>
#include <algorithm>
#include <common/fdnotifier.h>

extern FdNotifierRList g_fdnotifier_rlist;

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

FdNotifier* FdNotifier::create( int fd , int flags )
{
	FdNotifier* nf = new FdNotifier(fd, flags);

	RefPtr<FdNotifier> rfd (nf);

	g_fdnotifier_rlist.push_back(rfd);

	return nf;
}

void FdNotifier::destroy( FdNotifier* fn )
{
	//_dbg("destroying fdnotifier");

	FdNotifierRList::iterator i = g_fdnotifier_rlist.begin();

	while( i != g_fdnotifier_rlist.end() )
	{
		if( i->get() == fn )
		{
			FdNotifierRList::iterator n = i;
			n++;
			g_fdnotifier_rlist.erase(i);
			i = n;
		}
		else
		{
			++i;
		}
	}
}

FdNotifier::FdNotifier( int fd, int flags )
:	pvt(new Private)
{
	pvt->fd = fd;
	pvt->flags = flags;
	pvt->state = 0;
	pvt->data = NULL;
}

FdNotifier::~FdNotifier()
{
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
