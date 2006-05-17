# Autojunk script for Eeple

AC_PREREQ(2.59)
AC_INIT([libeeple], 0.1.0, [shackan@gmail.com])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER([include/eeple/config.h])

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


#  Check the operating system
#case "${target_os}" in
#	"")
#		AC_MSG_WARN([Couldn't read operating system type]);;
#	*mingw32* | *cygwin*)
#		AC_DEFINE([PLATFORM_WIN],,[We're on a Windows platform]);;
#	*)
#		;;
#esac

AC_OUTPUT([Makefile src/Makefile eeple-0.1.pc])
