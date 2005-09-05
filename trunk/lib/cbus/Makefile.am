
PACKAGES = sigc++-2.0 dbus-1

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include `pkg-config --cflags $(PACKAGES)`
AM_LDFLAGS =  -L$(srcdir)/../common `pkg-config --libs $(PACKAGES)`
AM_CXXFLAGS = -D_DEBUG -DDBUS_API_SUBJECT_TO_CHANGE

noinst_LIBRARIES = libcbus.a
libcbus_a_SOURCES = ./include/cbus/cbusdebug.h ./include/cbus/cbuserror.h ./include/cbus/cbusfilter.h ./include/cbus/cbus.h ./include/cbus/cbusinterface.h ./include/cbus/cbusmessage.h ./include/cbus/cbusmonitor.h ./include/cbus/cbusobject.h ./include/cbus/cbusservice.h ./include/cbus/cbusserver.h ./src/cbusservice.cpp ./src/cbusserver.cpp ./src/cbuspendingcall.cpp ./src/cbusobject.cpp ./src/cbusmonitor.cpp ./src/cbusmessage.cpp ./src/cbusinterface.cpp ./src/cbuserror.cpp ./src/cbusconnection.cpp
