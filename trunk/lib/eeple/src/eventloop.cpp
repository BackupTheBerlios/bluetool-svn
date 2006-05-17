#include <eeple/debug.h>
#include <eeple/eventloop.h>
#include <eeple/timeout.h>
#include <eeple/fdnotifier.h>

#include <exception>
#include <iostream>

#include <sys/poll.h>
#include <sys/time.h>

FdNotifierPList	g_fdnotifier_plist;
TimeoutPList	g_timeout_plist;

EventLoop::EventLoop()
:	_looping(false)
{}

void EventLoop::enter()
{
	Dbg::log_info("entering event loop");

	_looping = true;

	while(_looping)
	{	
		int nfd = g_fdnotifier_plist.size();

		pollfd fds[ nfd ];

		int a = 0;
		FdNotifierPList::iterator fit = g_fdnotifier_plist.begin();

		while( fit != g_fdnotifier_plist.end() )
		{
		//	if( (*fit)->fd() > 0 && (*fit)->flags() )
			{
				fds[a].fd	= (*fit)->fd();
				fds[a].events	= (*fit)->flags();
				fds[a].revents	= 0;

		//		++a;
			}
			++fit; ++a;
		}
		//nfd = a;

		//get nearest timeout
		int wait = 100;

		int returned = poll(fds, nfd, wait);

		if( returned > 0 ){ Dbg::log_info("%d descriptor%sready",returned, returned>1?"s ":" "); }

		TimeoutPList::iterator tit = g_timeout_plist.begin();
		
		while( tit != g_timeout_plist.end() )
		{
			/* if the timeout expired, its owner might decide
			   to destroy it, thus invalidating the pointer,
			   that's why we save it beforehand
			*/
			TimeoutPList::iterator tit2 = tit;
			tit2++;
			if( (*tit)->update() )
			{
				(*tit)->timed_out(**(tit));
			}
			tit=tit2;
		}

		int i = 0;
		int checked = 0;
		while( i < nfd )
		{
			FdNotifierPList::iterator fit = g_fdnotifier_plist.begin();
			while( fit != g_fdnotifier_plist.end() )
			{	
				if( (*fit)->fd() == fds[i].fd )
				{
					(*fit)->state( fds[i].revents );
					
					try
					{
						if( fds[i].revents & POLLIN )
						{
							(*fit)->can_read( **(fit) );
						}
						if( fds[i].revents & POLLOUT )
						{
							(*fit)->can_write( **(fit) );
						}
					}
					catch( std::exception& e )
					{
						Dbg::log_error("Uncaught exception in event loop: %s", e.what());
						//(*fit)->fd(-1);
					}
					if( fds[i].revents & POLLERR || fds[i].revents & POLLHUP || fds[i].revents == 32 ) // ?!?!?!?!?
					{
						/* remove it NOW or we enter an infinite loop
						*/
						Dbg::log_warn("fds[%d].revents = %d, removing..",i,fds[i].revents);

						FdNotifierPList::iterator next = fit;
						++next;
						g_fdnotifier_plist.erase(fit);

						fit = next;

						++checked;
						//++i;
						continue;
					}
					++checked;
					//++i;
				}
				++fit;
			}
			++i;
		}
	}
	Dbg::log_info("leaving event loop");
}

void EventLoop::leave()
{
	_looping = false;
}

