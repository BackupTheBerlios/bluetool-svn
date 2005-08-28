PACKAGES = sigc++-2.0 dbus-1

AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../lib/common/include -I$(srcdir)/../../lib/bluetool/include -I$(srcdir)/../../lib/cbus/include `pkg-config --cflags $(PACKAGES)` $(PYTHON_CPPFLAGS) 
AM_LDFLAGS =  `pkg-config --libs $(PACKAGES)` $(PYTHON_LDFLAGS) 
AM_CXXFLAGS = -ggdb -DDBUS_API_SUBJECT_TO_CHANGE -D_DEBUG

bin_PROGRAMS = bluetool-daemon

bluetool_daemon_SOURCES = ./src/hci/hci_device.h ./src/sdp/sdp_browser.h ./src/sdp/sdp_record.h ./src/services/btool_service.h ./src/services/btool_service_loader.h ./src/services/btool_service_database.h ./src/btool_device.h ./src/bluedebug.h ./src/btool_device_manager.h ./src/btool_names.h ./src/btool_root_service.h ./src/hci/hci_device.cpp ./src/sdp/sdp_browser.cpp ./src/sdp/sdp_record.cpp ./src/services/btool_service_loader.cpp ./src/services/btool_service.cpp ./src/services/btool_service_database.cpp ./src/main.cpp ./src/btool_device.cpp ./src/btool_device_manager.cpp ./src/btool_root_service.cpp

bluetool_daemon_LDADD = ../../lib/common/libshack.a ../../lib/bluetool/libbluetool.a ../../lib/cbus/libcbus.a 
