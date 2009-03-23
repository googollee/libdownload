# Automake file
AUTOMAKE_OPTIONS = foreign

AM_CFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/src/
AM_CPPFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/src/
AM_CXXFLAGS = -g -O0 -Wall -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/src/

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
BitMap_unittest_SOURCES = \
	utility/BitMap_unittest.cpp \
	$(top_srcdir)/src/utility/BitMap.h \
	$(top_srcdir)/src/utility/BitMap.cpp
BitMap_unittest_CPPFLAGS =
BitMap_unittest_LDADD = \
    gtest/lib/libgtest_main.la

TESTS += File_unittest
check_PROGRAMS += File_unittest
File_unittest_SOURCES = \
	utility/File_unittest.cpp \
	$(top_srcdir)/src/utility/File.h \
	$(top_srcdir)/src/utility/FilePosixApi.h
File_unittest_CPPFLAGS =
File_unittest_LDADD = \
    gtest/lib/libgtest_main.la

TESTS += SimpleXmlParser_unittest
check_PROGRAMS += SimpleXmlParser_unittest
SimpleXmlParser_unittest_SOURCES = \
	utility/SimpleXmlParser_unittest.cpp \
	$(top_srcdir)/src/utility/SimpleXmlParser.h \
	$(top_srcdir)/src/utility/SimpleXmlParser.cpp
SimpleXmlParser_unittest_CPPFLAGS = $(GLIB_CFLAGS)
SimpleXmlParser_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(GLIB_LIBS)

TESTS += protocols/HttpProtocol_unittest.sh
check_PROGRAMS += HttpProtocol_unittest
HttpProtocol_unittest_SOURCES = \
	protocols/HttpProtocol_unittest.cpp \
	$(top_srcdir)/src/protocols/ProtocolBase.h \
	$(top_srcdir)/src/protocols/HttpProtocol.h \
	$(top_srcdir)/src/protocols/HttpProtocolImpl.h \
	$(top_srcdir)/src/protocols/HttpProtocol.cpp
HttpProtocol_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
HttpProtocol_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)

TESTS += ProtocolFactory_unittest
check_PROGRAMS += ProtocolFactory_unittest
ProtocolFactory_unittest_SOURCES = \
	ProtocolFactory_unittest.cpp \
	$(top_srcdir)/src/ProtocolFactory.h \
	$(top_srcdir)/src/ProtocolFactory.cpp
ProtocolFactory_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
ProtocolFactory_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/protocols/libprotocols.la \
    $(top_srcdir)/src/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)

TESTS += DownloadManager_unittest
check_PROGRAMS += DownloadManager_unittest
DownloadManager_unittest_SOURCES = DownloadManager_unittest.cpp
DownloadManager_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
DownloadManager_unittest_LDADD = \
    gtest/lib/libgtest_main.la \
    $(top_srcdir)/src/libdownload.la \
    $(top_srcdir)/src/protocols/libprotocols.la \
    $(top_srcdir)/src/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)