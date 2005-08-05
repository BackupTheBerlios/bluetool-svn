#include <algorithm>
#include "fdnotifier.h"

extern FdNotifierPList g_fdnotifier_plist;

struct FdNotifier::Private
{
	int	fd;
	int	flags;
	void*	data;
};

FdNotifier::FdNotifier( int fd, int flags )
{
	pvt = new Private;
	pvt->fd = fd;
	pvt->flags = flags;
	pvt->data = NULL;

	g_fdnotifier_plist.push_back(this);
}

FdNotifier::~FdNotifier()
{
	if(noref())
	{
		FdNotifierPList::iterator i =
			std::find(
				g_fdnotifier_plist.begin(),
				g_fdnotifier_plist.end(),
				this
			);

		if(i != g_fdnotifier_plist.end()) 
			g_fdnotifier_plist.erase(i);
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
