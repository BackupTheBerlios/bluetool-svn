
PACKAGES = sigc++-2.0

AM_CPPFLAGS = -I$(srcdir)/include `pkg-config --cflags $(PACKAGES)`
AM_LDFLAGS = `pkg-config --libs $(PACKAGES)`
AM_CXXFLAGS = -ggdb -D_DEBUG

noinst_LIBRARIES = libshack.a
libshack_a_SOURCES = ./src/timeout.cpp ./src/fdnotifier.cpp ./src/eventloop.cpp ./src/debug.cpp ./src/configfile.cpp ./src/thread.cpp ./include/common/bytevector.h ./include/common/configfile.h ./include/common/debug.h ./include/common/eventloop.h ./include/common/fdnotifier.h ./include/common/refcnt.h ./include/common/refptr.h ./include/common/socket.h ./include/common/timeout.h ./include/common/types.h ./include/common/unixsocket.h ./include/common/thread.h ./include/common/mutex.h
