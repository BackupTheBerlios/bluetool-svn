#ifndef __HAL_DEBUG_H
#define __HAL_DEBUG_H

#include "../common/debug.h"

#ifndef HAL_DEBUG
#	define hal_dbg(format, ...)	fprintf(stderr, " hal|");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n");
#else
#	define hal_dbg(...)
#endif

#endif//__HAL_DEBUG_H
