INCLUDES = $(sigcxx_CFLAGS) -I$(top_srcdir)/include 

HEADER_DIR  = $(top_srcdir)/include/eeple

HEADER_FILES = \
	$(HEADER_DIR)/config.h \
	$(HEADER_DIR)/bytebuffer.h \
	$(HEADER_DIR)/debug.h \
	$(HEADER_DIR)/error.h \
	$(HEADER_DIR)/eventloop.h \
	$(HEADER_DIR)/fdnotifier.h \
	$(HEADER_DIR)/refcnt.h \
	$(HEADER_DIR)/refptr.h \
	$(HEADER_DIR)/refptr_impl.h \
	$(HEADER_DIR)/socket.h \
	$(HEADER_DIR)/timeout.h \
	$(HEADER_DIR)/types.h \
	$(HEADER_DIR)/unixsocket.h

lib_includedir=$(includedir)/eeple-0.1/eeple
lib_include_HEADERS = $(HEADER_FILES)

lib_LTLIBRARIES = libeeple-0.1.la
libeeple_0_1_la_SOURCES = $(HEADER_FILES) debug.cpp  eventloop.cpp  fdnotifier.cpp  timeout.cpp
libeeple_0_1_la_LIBADD = $(sigcxx_LIBS)
