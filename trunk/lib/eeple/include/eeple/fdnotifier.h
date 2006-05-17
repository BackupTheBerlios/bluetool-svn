#ifndef __EEPLE_FD_NOTIFIER_H
#define __EEPLE_FD_NOTIFIER_H

#include <sys/poll.h>
#include <sigc++/sigc++.h>
#include <list>
#include "refptr.h"

class FdNotifier;

typedef sigc::signal<void, FdNotifier&> Readable;
typedef sigc::signal<void, FdNotifier&> Writable;

class FdNotifier
{
public:

	FdNotifier( int fd, int flags );

	~FdNotifier();

private:

	FdNotifier( const FdNotifier& );

//	const FdNotifier& operator = ( const FdNotifier& );

public:
	int fd() const;
	int flags() const;
	int state() const;
	
	void fd( int );
	void flags( int );
	void state( int );

	void data( void* );
	void* data() const;

	Readable can_read;
	Writable can_write;

private:
	struct Private;
	RefPtr<Private> pvt;
};

typedef std::list<FdNotifier*> FdNotifierPList;
//typedef std::list< RefPtr<FdNotifier> > FdNotifierRList;

#endif//__EEPLE_FD_NOTIFIER_H
