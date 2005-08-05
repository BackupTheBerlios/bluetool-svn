#ifndef __CBUS_FD_NOTIFIER_H
#define __CBUS_FD_NOTIFIER_H

#include <sys/poll.h>
#include <list>
#include <sigc++/sigc++.h>
#include "refcnt.h"

class FdNotifier;

typedef sigc::signal<void, FdNotifier&> Readable;
typedef sigc::signal<void, FdNotifier&> Writable;

class FdNotifier : public RefCnt
{
public:
	FdNotifier( int fd = -1, int flags = 0 );
	~FdNotifier();

//	const FdNotifier& operator = ( const FdNotifier& );

	int fd() const;
	int flags() const;
	
	void fd( int );
	void flags( int );

	void data( void* );
	void* data() const;

	Readable can_read;
	Writable can_write;

private:
	struct Private;
	Private* pvt;
};

typedef std::list<FdNotifier> FdNotifierList;
typedef std::list<FdNotifier*> FdNotifierPList;

#endif//__CBUS_FD_NOTIFIER_H
