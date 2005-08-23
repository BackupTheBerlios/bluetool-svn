#ifndef __HCI_DEBUG_H
#define __HCI_DEBUG_H

#include <common/debug.h>

//#ifdef HCI_DEBUG
#	define hci_dbg(format, ...)	debugp(" hci",format,##__VA_ARGS__);
#	define hci_dbg_enter()		dbg_enter(" hci")
#	define hci_dbg_leave()		dbg_leave(" hci")
//#else
//#	define hci_dbg(...)
//#endif

#endif//__HCI_DEBUG_H
