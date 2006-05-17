#ifndef __DEBUG_H
#define __DEBUG_H

#include <sigc++/sigc++.h>
#include <stdarg.h>
#include <cstdio>
#include <syslog.h>

namespace Dbg
{

typedef void (*LogFunction)(char*, ...);

extern LogFunction log_info;
extern LogFunction log_warn;
extern LogFunction log_error;

class FrameLogger
{
public:
	FrameLogger(const char* function_name);

	~FrameLogger();
private:
	const char* _fn;
};

}

#ifdef _DEBUG
#define TRACE_CALL volatile Dbg::FrameLogger(__PRETTY_FUNCTION__) __f__;
#else
#define TRACE_CALL
#endif

#endif//__DEBUG_H

