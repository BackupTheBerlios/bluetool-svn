#ifndef __CBUS_EVENT_LOOP_H
#define __CBUS_EVENT_LOOP_H

class EventLoop
{
public:
	EventLoop();

	void enter();

	void leave();

private:

	bool		_looping;
};

#endif//__CBUS_EVENT_LOOP_H
