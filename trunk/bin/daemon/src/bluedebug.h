#ifndef __BLUE_DEBUG_H
#define __BLUE_DEBUG_H

#include <common/debug.h>

#ifdef _DEBUG
#	define blue_dbg(format, ...){	fprintf(stderr, "blue|");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n"); }
#else
#	define blue_dbg(...)
#endif

#endif
