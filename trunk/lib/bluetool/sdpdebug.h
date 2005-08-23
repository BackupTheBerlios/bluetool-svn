#ifndef __SDP_DEBUG_H
#define __SDP_DEBUG_H

#include <common/debug.h>

#ifdef SDP_DEBUG
#	define sdp_dbg(format, ...){	fprintf(stderr, " sdp|");		\
					fprintf(stderr, format, ##__VA_ARGS);	\
					fprintf(stderr, "\n"); }
#else
#	define sdp_dbg(...)
#endif

#endif//__HCI_DEBUG_H
