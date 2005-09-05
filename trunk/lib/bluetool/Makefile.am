PACKAGES = sigc++-2.0

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../common/include `pkg-config --cflags $(PACKAGES)`
AM_LDFLAGS = -L$(srcdir)/../common `pkg-config --libs $(PACKAGES)`
AM_CXXFLAGS = -D_DEBUG

noinst_LIBRARIES = libbluetool.a
libbluetool_a_SOURCES = ./include/bluetool/bdaddr.h ./include/bluetool/hciconnection.h ./include/bluetool/hcidebug.h ./include/bluetool/hcidevice.h ./include/bluetool/hcievent.h ./include/bluetool/hcierror.h ./include/bluetool/hcifilter.h ./include/bluetool/hcisocket.h ./include/bluetool/l2csocket.h ./include/bluetool/sdpclient.h ./include/bluetool/sdpdebug.h ./include/bluetool/sdperror.h ./include/bluetool/sdprecord.h ./src/linux/bdaddr.cpp ./src/linux/hciconnection.cpp ./src/linux/hcidevice.cpp ./src/linux/hcierror.cpp ./src/linux/hcifilter.cpp ./src/linux/hcisocket.cpp ./src/linux/l2csocket.cpp ./src/linux/sdpclient.cpp ./src/linux/sdperror.cpp ./src/linux/sdprecord.cpp 
