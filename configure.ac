#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.6])
AC_INIT([libdownload], [0.0.1], [googollee@gmail.com])
AC_CONFIG_SRCDIR([./COPYING])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

AC_MSG_RESULT(libdownload version: $LIBDOWNLOAD_INTERFACE_VERSION_NO)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Option to enable debugging
AC_ARG_ENABLE(debug,
              [AC_HELP_STRING([--enable-debug], [Debugging [default=no]])],
              [with_debug=yes], [with_debug=no])
AC_MSG_RESULT(enable debug... $with_debug)
if test "x$with_debug" == "xyes" ; then
  CPPFLAGS=" -ggdb -O0 -D_DEBUG -Wall -Wextra"
else
  CPPFLAGS=" -O2 -Wno-deprecated -Wall -Wextra"
fi

# Checks for libraries.

# check for glib
PKG_CHECK_MODULES(GLIB, "glib-2.0")
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)
AC_MSG_RESULT(GLIB_CFLAGS... $GLIB_CFLAGS)
AC_MSG_RESULT(GLIB_LIBS... $GLIB_LIBS)

# check for curl
LIBCURL_CHECK_CONFIG([yes], [7.18.0], ,AC_MSG_ERROR([We could not detect the libcurl libraries (version 7.19.0 or higher). If you have a staged libcurl library (still not installed) please specify the PATH with --with-libcurl option.]))
AC_SUBST(LIBCURL_CPPFLAGS)
LIBCURL_LIBS=$LIBCURL
AC_SUBST(LIBCURL_LIBS)
AC_MSG_RESULT(LIBCURL_CFLAGS... $LIBCURL_CPPFLAGS)
AC_MSG_RESULT(LIBCURL_LIBS... $LIBCURL_LIBS)

# check for boost
AX_BOOST_BASE([1.34.0])
AX_BOOST_SIGNALS()

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stddef.h stdlib.h string.h strings.h sys/time.h sys/timeb.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([dup2 getcwd getpagesize gettimeofday memset mkdir munmap regcomp strcasecmp strchr strdup strerror strrchr strstr strtol])

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 lib/utility/Makefile
                 unittest/Makefile
                 unittest/gtest/Makefile])
AC_OUTPUT
