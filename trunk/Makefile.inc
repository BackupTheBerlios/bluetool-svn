CC	= g++
CPP	= cpp

DBGFLAGS	= -D_DEBUG -ggdb

#		add -DDBUS_LEGACY to compile with older versions of dbus
CFLAGS		= -Wall -DDBUS_API_SUBJECT_TO_CHANGE $(DBGFLAGS)
PACKAGES	= dbus-1 sigc++-2.0
INCLUDEDIR	= `pkg-config --cflags $(PACKAGES)` -I/usr/include/python2.4
ALLCFLAGS	= $(CFLAGS) $(INCLUDEDIR)
LDFLAGS		= `pkg-config --libs $(PACKAGES)` -ggdb -lbluetooth -lpython2.4

RULEFILE	= Makefile.rules

#MFLAGS = --print-directory

#
#	Implicit rule to create objects from C++ sources
#
%.o : %.cpp
	$(CC) $< -c -o $@ $(ALLCFLAGS)

