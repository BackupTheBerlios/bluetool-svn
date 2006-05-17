/*
 *
 *  C-Bus - C++ bindings for DBus
 *
 *  Copyright (C) 2005-2006  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __CBUS_DEBUG_H
#define __CBUS_DEBUG_H

#include <eeple/debug.h>

#define cbus_dbg(format, ... ){ Dbg::log_info("dbus: "format,##__VA_ARGS__); }

#if 0
#ifdef _DEBUG
#	define cbus_dbg(format, ...){	fprintf(stderr, "dbus|");		\
					fprintf(stderr, format, ##__VA_ARGS__);	\
					fprintf(stderr, "\n");	}
#else
#	define cbus_dbg(format, ...)
#endif
#endif

#endif
