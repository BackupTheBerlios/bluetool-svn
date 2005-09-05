#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdarg.h>
#include <cstdio>
#include <syslog.h>

#ifdef _DEBUG

	extern void (*vdebugp)( const char*, va_list );

	extern void debugp( const char*, const char*, ... );

	extern int nesting;

//#	define dbg_enter(prefix)	debugp(prefix,"> %s ("__FILE__":%d)",__PRETTY_FUNCTION__,__LINE__);
//#	define dbg_leave(prefix)	debugp(prefix,"< %s ("__FILE__":%d)",__PRETTY_FUNCTION__,__LINE__);
#	define dbg_enter(prefix)	{	++nesting;					\
						debugp(prefix,"> %s",__PRETTY_FUNCTION__);	\
					}
#	define dbg_leave(prefix)	{	debugp(prefix,"< %s",__PRETTY_FUNCTION__);	\
						--nesting;					\
					}
#	define _dbg(format, ...)	debugp("    ",format,##__VA_ARGS__);
#else
#	define dbg_enter(prefix)
#	define dbg_leave(prefix)
#	define _dbg(format, ...)
#endif

#endif//__DEBUG_H

