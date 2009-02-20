# Automake file
AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include
AM_CPPFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include
AM_CXXFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include

SUBDIRS = gtest

# Bulid rules for samples and tests. Automake's naming for some of
# these variables isn't terribly obvious, so this is a brief
# reference:
#
# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

TESTS =
check_PROGRAMS =

TESTS += BitMap_unittest
check_PROGRAMS += BitMap_unittest
BitMap_unittest_SOURCES = utility/BitMap_unittest.cpp \
                          ../src/utility/BitMap.h ../src/utility/BitMap.cpp
BitMap_unittest_CPPFLAGS = -I../src/
BitMap_unittest_LDADD = gtest/lib/libgtest_main.la

TESTS += File_unittest
check_PROGRAMS += File_unittest
File_unittest_SOURCES = utility/File_unittest.cpp
File_unittest_CPPFLAGS = -I../src/
File_unittest_LDADD = gtest/lib/libgtest_main.la

TESTS += HttpProtocol_unittest
check_PROGRAMS += HttpProtocol_unittest
HttpProtocol_unittest_SOURCES = protocol/HttpProtocol_unittest.cpp \
                                ../src/protocols/http/HttpProtocol.h ../src/protocols/http/HttpProtocol.cpp \
                                ../src/utility/BitMap.h ../src/utility/BitMap.cpp
HttpProtocol_unittest_CPPFLAGS = -I../src/ ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS}
HttpProtocol_unittest_LDADD = gtest/lib/libgtest_main.la ${LIBCURL_LIBS} ${BOOST_LDFLAGS} ${BOOST_SIGNALS_LIB}
