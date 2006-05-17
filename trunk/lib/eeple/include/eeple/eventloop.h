#ifndef __EEPLE_EVENT_LOOP_H
#define __EEPLE_EVENT_LOOP_H

class EventLoop
{
public:
	EventLoop();

	void enter();

	void leave();

private:

	bool	_looping;
};

#endif//__EEPLE_EVENT_LOOP_H
