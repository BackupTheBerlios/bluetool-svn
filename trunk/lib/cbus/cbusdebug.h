#ifndef __CBUS_DEBUG_H
#define __CBUS_DEBUG_H

#include "../common/debug.h"

#ifdef _DEBUG
#	define cbus_dbg(format, ...){	fprintf(stderr, "dbus|");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n");	}
#else
#	define cbus_dbg(format, ...)
#endif

#endif
