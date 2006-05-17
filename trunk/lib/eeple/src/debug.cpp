#include <eeple/debug.h>
#include <stdarg.h>
#include <stdio.h>

static void _log_info_default(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");

	va_end(args);
}

static void _log_error_default(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);
}

Dbg::LogFunction Dbg::log_info = _log_info_default;
Dbg::LogFunction Dbg::log_warn = _log_error_default;
Dbg::LogFunction Dbg::log_error = _log_error_default;

using namespace Dbg;

FrameLogger::FrameLogger(const char* function_name)
: _fn(function_name)
{
	log_info(" > %s",_fn);
}
FrameLogger::~FrameLogger()
{
	log_info(" < %s",_fn);
}
