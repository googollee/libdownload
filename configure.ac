#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libdownload unit test], [0.0.1], [googollee@gmail.com])
AC_CONFIG_SRCDIR([./COPYING])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(libdownload_unittest, 0.0.1)
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

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
                 unittest/Makefile
                 unittest/gtest/Makefile
                 unittest/utility/Makefile])
AC_OUTPUT