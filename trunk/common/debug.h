#ifndef __DEBUG_H
#define __DEBUG_H

#include <cstdio>

#ifdef _DEBUG
#	define _dbg(format, ...)	fprintf(stderr, "    |");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n");
#else
#	define _dbg(format, ...)
#endif

#endif//__DEBUG_H
