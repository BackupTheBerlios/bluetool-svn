EXTRA_DIST = README

INCLUDES = $(sigcxx_CFLAGS) $(dbus_CFLAGS) $(eeple_CFLAGS) -I$(top_srcdir)/include 
AM_CPPFLAGS = -DDBUS_API_SUBJECT_TO_CHANGE
AM_CFLAGS = -g -O0

noinst_PROGRAMS = echo_server

echo_server_SOURCES = server.cpp server.h
echo_server_LDADD = $(sigcxx_LIBS) $(dbus_LIBS) $(eeple_LIBS) $(top_srcdir)/src/libcbus-0.1.la
