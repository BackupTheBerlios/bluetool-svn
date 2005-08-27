#ifndef __CBUS_FD_NOTIFIER_H
#define __CBUS_FD_NOTIFIER_H

#include <sys/poll.h>
#include <list>
#include <sigc++/sigc++.h>
#include "refptr.h"

class FdNotifier;

typedef sigc::signal<void, FdNotifier&> Readable;
typedef sigc::signal<void, FdNotifier&> Writable;

class FdNotifier
{
public:
	static FdNotifier* create( int fd = -1, int flags = 0 );
	static void destroy( FdNotifier* fn );

	~FdNotifier();

private:
	FdNotifier( int fd, int flags );

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

typedef std::list<FdNotifier> FdNotifierList;
typedef std::list< RefPtr<FdNotifier> > FdNotifierRList;

#endif//__CBUS_FD_NOTIFIER_H
