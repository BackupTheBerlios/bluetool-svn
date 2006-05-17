INCLUDES = $(sigcxx_CFLAGS) $(dbus_CFLAGS) $(eeple_CFLAGS) -I$(top_srcdir)/include 
AM_CPPFLAGS = -DDBUS_API_SUBJECT_TO_CHANGE

HEADER_DIR  = $(top_srcdir)/include/cbus
HEADER_FILES = \
	$(HEADER_DIR)/config.h \
	$(HEADER_DIR)/cbusconnection.h \
	$(HEADER_DIR)/cbusdebug.h \
	$(HEADER_DIR)/cbuserror.h \
	$(HEADER_DIR)/cbusfilter.h \
	$(HEADER_DIR)/cbus.h \
	$(HEADER_DIR)/cbusinterface.h \
	$(HEADER_DIR)/cbusmessage.h \
	$(HEADER_DIR)/cbusmonitor.h \
	$(HEADER_DIR)/cbusobject.h \
	$(HEADER_DIR)/cbuspendingcall.h \
	$(HEADER_DIR)/cbusserver.h

lib_includedir=$(includedir)/cbus-0.1/cbus
lib_include_HEADERS = $(HEADER_FILES)

lib_LTLIBRARIES = libcbus-0.1.la
libcbus_0_1_la_SOURCES = $(HEADER_FILES) cbusconnection.cpp cbusinterface.cpp cbusmonitor.cpp cbuspendingcall.cpp  cbuserror.cpp cbusmessage.cpp cbusobject.cpp cbusserver.cpp
libcbus_0_1_la_LIBADD = $(sigcxx_LIBS) $(dbus_LIBS) $(eeple_LIBS)
