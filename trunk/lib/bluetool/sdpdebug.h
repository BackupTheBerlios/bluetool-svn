#ifndef __SDP_DEBUG_H
#define __SDP_DEBUG_H

#define SDP_DEBUG

#include <common/debug.h>

#ifdef SDP_DEBUG
#	define sdp_dbg(format, ...){	fprintf(stderr, " sdp|");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n"); }
#	define sdp_dbg_enter()		dbg_enter(" sdp")
#	define sdp_dbg_leave()		dbg_leave(" sdp")
#else
#	define sdp_dbg(...)
#endif

#endif//__HCI_DEBUG_H
