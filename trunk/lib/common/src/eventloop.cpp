#include <common/debug.h>
#include <common/eventloop.h>
#include <common/timeout.h>
#include <common/fdnotifier.h>

#include <exception>
#include <iostream>

#include <sys/poll.h>
#include <sys/time.h>

FdNotifierRList	g_fdnotifier_rlist;
TimeoutRList	g_timeout_rlist;

EventLoop::EventLoop()
:	_looping(false)
{}

void EventLoop::enter()
{
	_dbg("entering loop");

	_looping = true;

	while(_looping)
	{	
		int nfd = g_fdnotifier_rlist.size();

		pollfd fds[ nfd ];

		int a = 0;
		FdNotifierRList::iterator fit = g_fdnotifier_rlist.begin();

		while( fit != g_fdnotifier_rlist.end() )
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
/*
		TimeoutRList::iterator ti = g_timeout_rlist.begin();

		while( ti != g_timeout_rlist.end() )
		{
			if( (*ti)->timeslice() < wait )
				wait = (*ti)->timeslice();

			++ti;
		}*/

//		timeval before;
//		gettimeofday(&before, NULL);

		int returned = poll(fds, nfd, wait);

		if( returned > 0 ){_dbg("%d descriptor%sready",returned, returned>1?"s ":" ");}
		
//		timeval after;
//		gettimeofday(&after, NULL);

//		int time_delta =	((after.tv_sec*1000 + after.tv_usec/1000.0)
//					- (before.tv_sec*1000 + before.tv_usec/1000.0));

		TimeoutRList::iterator tit = g_timeout_rlist.begin();
		
		while( tit != g_timeout_rlist.end() )
		{
			/*int newslice = (*tit)->timeslice() - time_delta;
			
			if( newslice <= 0 )
			{
				(*tit)->timeslice( (*tit)->interval() );

				(*tit)->timed_out( *(*tit) );
			}
			else
			{
				(*tit)->timeslice( newslice );
			}*/

			/* if the timeout expired, its owner might decide
			   to destroy it, thus invalidating the pointer,
			   that's why we save it beforehand
			*/
			TimeoutRList::iterator tit2 = tit;
			tit2++;
			if( (*tit)->update() )
			{
				(*tit)->timed_out(*(tit->get()));
			}
			tit=tit2;
		}

		int i = 0;
		int checked = 0;
		while( i < nfd )
		{
			FdNotifierRList::iterator fit = g_fdnotifier_rlist.begin();
			while( fit != g_fdnotifier_rlist.end() )
			{	
				if( (*fit)->fd() == fds[i].fd )
				{
					(*fit)->state( fds[i].revents );
					
					try
					{

						if( fds[i].revents & POLLIN )
						{
							(*fit)->can_read( *(fit->get()) );
						}
						if( fds[i].revents & POLLOUT )
						{
							(*fit)->can_write( *(fit->get()) );
						}

					}
					catch( std::exception& e )
					{
						_dbg("Uncaught exception in event loop: %s", e.what());
						//(*fit)->fd(-1);
					}
					if( fds[i].revents & POLLERR || fds[i].revents & POLLHUP || fds[i].revents == 32 ) // ?!?!?!?!?
					{
						/* remove it NOW or we enter an infinite loop
						*/
						_dbg("fds[%d].revents = %d, removing..",i,fds[i].revents);

						FdNotifierRList::iterator next = fit;
						++next;
						FdNotifier::destroy(fit->get());
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
	_dbg("leaving loop");
}










#if 0
		pollfd fds[256];	//TODO! this is just an indicative value
		int fdi = 0;
		MonitorList::iterator m = _monitors.begin();
		while(m != _monitors.end())
		{
			WatchList::iterator w = (*m)->_watches.begin();
			while(w != (*m)->_watches.end())
			{
				fds[fdi].fd		= w->fd;
				fds[fdi].events		= w->events;
				fds[fdi].revents	= 0;

				w++;	fdi++;
			}
			++m;
		}

		timeval before;
		gettimeofday(&before, NULL);

		/*	let the poll begin!
		*/
		int readyfds = poll(fds, fdi, 10);	//todo: a finer resolution would be better, but I don't care
		
		timeval after;
		gettimeofday(&after, NULL);

		double time_delta =	((after.tv_sec*1000000.0 + after.tv_usec)
					- (before.tv_sec*1000000.0 + before.tv_usec));

		/*	how much time has passed?
			adjust the timeouts accordingly
		*/
		MonitorList::iterator mt = _monitors.begin();
		while( mt != _monitors.end() )
		{
			(*mt)->adjust_timeouts(time_delta);
		}

		/*	now handle the file descriptors
		*/
		if( readyfds > 0 )
		{
			int i = 0;
			while( i < readyfds )
			{
				if( fds[i].revents )
				{	
					/*	find the watch handling this file descriptor
					*/
					MonitorList::iterator mf = _monitors.begin();
					while( mf != _monitors.end() )
					{
						if( (*mf)->poll_result(fds[i]) )
							break;
						++mf;
					}
					//TODO: looping this way is slow, how to improve that ?
				}
				++i;
			}
		}
	}
}
#endif

void EventLoop::leave()
{
	_looping = false;
}

