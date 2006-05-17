# Autojunk script for libcbus

AC_PREREQ(2.59)
AC_INIT([libcbus], 0.1.0, [shackan@gmail.com])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER([include/cbus/config.h])

AC_CANONICAL_HOST


# Check for programs

AC_LANG_CPLUSPLUS

AC_PROG_CC
AC_PROG_CXX

AM_ENABLE_STATIC
AM_DISABLE_SHARED
AM_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG


# Check for dependencies

SIGC_REQUIRED_VERSION=2.0
PKG_CHECK_MODULES([sigcxx], sigc++-2.0 >= $SIGC_REQUIRED_VERSION,,
	AC_MSG_ERROR([Make sure you have the latest SigC++ library]
	[http://libsigc.sourceforge.net/])
)
AC_SUBST(sigcxx_CFLAGS)
AC_SUBST(sigcxx_LIBS)

DBUS_REQUIRED_VERSION=0.60
PKG_CHECK_MODULES([dbus], dbus-1 >= $DBUS_REQUIRED_VERSION,,
	AC_MSG_ERROR([You need the DBus libraries (version 0.6 or better)]
	[http://freedesktop.org/wiki/Software_2fdbus])
)
AC_SUBST(dbus_CFLAGS)
AC_SUBST(dbus_LIBS)


PKG_CHECK_MODULES([eeple], eeple-0.1,,
	AC_MSG_ERROR([Eeple event library not found])
)
AC_SUBST(eeple_CFLAGS)
AC_SUBST(eeple_LIBS)

# Save processed files

AC_OUTPUT(Makefile src/Makefile tests/Makefile tests/server/Makefile cbus-0.1.pc)
