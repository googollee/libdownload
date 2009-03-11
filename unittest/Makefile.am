# Automake file
AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include
AM_CPPFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include
AM_CXXFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include

SUBDIRS = gtest

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src/

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
BitMap_unittest_SOURCES = utility/BitMap_unittest.cpp
BitMap_unittest_CPPFLAGS =
BitMap_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/utility/libutility.la

TESTS += File_unittest
check_PROGRAMS += File_unittest
File_unittest_SOURCES = utility/File_unittest.cpp
File_unittest_CPPFLAGS =
File_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/utility/libutility.la

TESTS += SimpleXmlParser_unittest
check_PROGRAMS += SimpleXmlParser_unittest
SimpleXmlParser_unittest_SOURCES = utility/SimpleXmlParser_unittest.cpp
SimpleXmlParser_unittest_CPPFLAGS = $(GLIB_CFLAGS)
SimpleXmlParser_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/utility/libutility.la \
    $(GLIB_LIBS)

TESTS += protocol/HttpProtocol_unittest.sh
check_PROGRAMS += HttpProtocol_unittest
HttpProtocol_unittest_SOURCES = protocol/HttpProtocol_unittest.cpp
HttpProtocol_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
HttpProtocol_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/protocols/libprotocols.la \
    $(top_srcdir)/src/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)
