#ifndef __HCI_DEBUG_H
#define __HCI_DEBUG_H

#include "../common/debug.h"

#ifdef HCI_DEBUG
#	define hci_dbg(format, ...)	fprintf(stderr, " hci|");		\
					fprintf(stderr, format, ##__VA_ARGS);	\
					fprintf(stderr, "\n");
#else
#	define hci_dbg(...)
#endif

#endif//__HCI_DEBUG_H
