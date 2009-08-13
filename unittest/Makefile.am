# Automake file
AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -g -O0 -Wall -Wextra -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/lib/
AM_CFLAGS = -g -O0 -Wall -Wextra -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/lib/
AM_CXXFLAGS = -g -O0 -Wall -Wextra -D_DEBUG -Igtest/$(srcdir) -Igtest/$(srcdir)/include -I$(top_srcdir) -I$(top_srcdir)/lib/

SUBDIRS = gtest

# Bulid rules for samples and tests. Automake's naming for some of
# these variables isn't terribly obvious, so this is a brief
# reference:
#
# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run

TESTS =
check_PROGRAMS =

TESTS += File_unittest
check_PROGRAMS += File_unittest
File_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/File.h \
	$(top_srcdir)/lib/utility/FilePosixApi.h \
	utility/File_unittest.cpp
File_unittest_CPPFLAGS =
File_unittest_LDADD = \
	gtest/lib/libgtest_main.la

TESTS += Allocator_unittest
check_PROGRAMS += Allocator_unittest
Allocator_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/Allocator.h \
	utility/Allocator_unittest.cpp
Allocator_unittest_CPPFLAGS =
Allocator_unittest_LDADD = \
	gtest/lib/libgtest_main.la

TESTS += LocalPtr_unittest
check_PROGRAMS += LocalPtr_unittest
LocalPtr_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/LocalPtr.h \
	utility/LocalPtr_unittest.cpp
LocalPtr_unittest_CPPFLAGS =
LocalPtr_unittest_LDADD = \
	gtest/lib/libgtest_main.la

TESTS += SocketManager_unittest
check_PROGRAMS += SocketManager_unittest
SocketManager_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/SocketManager.h \
	utility/SocketManager_unittest.cpp
SocketManager_unittest_CPPFLAGS =
SocketManager_unittest_LDADD = \
	gtest/lib/libgtest_main.la

TESTS += SimpleXmlParser_unittest
check_PROGRAMS += SimpleXmlParser_unittest
SimpleXmlParser_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/SimpleXmlParser.h \
	$(top_srcdir)/lib/utility/SimpleXmlParser.cpp \
	utility/SimpleXmlParser_unittest.cpp
SimpleXmlParser_unittest_CPPFLAGS = $(GLIB_CFLAGS)
SimpleXmlParser_unittest_LDADD = \
	gtest/lib/libgtest_main.la \
	$(GLIB_LIBS)

TESTS += BitMap_unittest
check_PROGRAMS += BitMap_unittest
BitMap_unittest_SOURCES = \
	$(top_srcdir)/lib/protocols/http/BitMap.h \
	$(top_srcdir)/lib/protocols/http/BitMap.cpp \
	protocols/BitMap_unittest.cpp
BitMap_unittest_CPPFLAGS =
BitMap_unittest_LDADD = \
	gtest/lib/libgtest_main.la

TESTS += protocols/HttpSession_unittest.sh
check_PROGRAMS += HttpSession_unittest
HttpSession_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/File.h \
	$(top_srcdir)/lib/utility/FilePosixApi.h \
	$(top_srcdir)/lib/utility/FileManager.h \
	$(top_srcdir)/lib/protocols/TaskBase.h \
	$(top_srcdir)/lib/protocols/TaskBase.cpp \
	$(top_srcdir)/lib/protocols/http/BitMap.h \
	$(top_srcdir)/lib/protocols/http/BitMap.cpp \
	$(top_srcdir)/lib/protocols/http/HttpConfigure.h \
	$(top_srcdir)/lib/protocols/http/HttpSession.h \
	$(top_srcdir)/lib/protocols/http/HttpSession.cpp \
	$(top_srcdir)/lib/protocols/http/HttpTask.h \
	protocols/HttpSession_unittest.cpp
HttpSession_unittest_CPPFLAGS = \
	${LIBCURL_CPPFLAGS} \
	${BOOST_CPPFLAGS}
HttpSession_unittest_LDADD = \
	gtest/lib/libgtest_main.la \
	${LIBCURL_LIBS} \
	${BOOST_LDFLAGS} \
	${BOOST_SIGNALS_LIB}

TESTS += HttpTask_unittest
check_PROGRAMS += HttpTask_unittest
HttpTask_unittest_SOURCES = \
	$(top_srcdir)/lib/utility/File.h \
	$(top_srcdir)/lib/utility/FilePosixApi.h \
	$(top_srcdir)/lib/utility/FileManager.h \
	$(top_srcdir)/lib/protocols/TaskBase.h \
	$(top_srcdir)/lib/protocols/TaskBase.cpp \
	$(top_srcdir)/lib/protocols/http/BitMap.h \
	$(top_srcdir)/lib/protocols/http/BitMap.cpp \
	$(top_srcdir)/lib/protocols/http/HttpConfigure.h \
	$(top_srcdir)/lib/protocols/http/HttpSession.h \
	$(top_srcdir)/lib/protocols/http/HttpSession.cpp \
	$(top_srcdir)/lib/protocols/http/HttpTask.h \
	$(top_srcdir)/lib/protocols/http/HttpTask.cpp \
	protocols/HttpTask_unittest.cpp
HttpTask_unittest_CPPFLAGS = \
	${LIBCURL_CPPFLAGS} \
	${BOOST_CPPFLAGS}
HttpTask_unittest_LDADD = \
	gtest/lib/libgtest_main.la \
	${LIBCURL_LIBS} \
	${BOOST_LDFLAGS} \
	${BOOST_SIGNALS_LIB}

#TESTS += protocols/HttpProtocol_unittest.sh
#check_PROGRAMS += HttpProtocol_unittest
#HttpProtocol_unittest_SOURCES = \
#    protocols/HttpProtocol_unittest.cpp \
#    $(top_srcdir)/lib/protocols/ProtocolBase.h \
#    $(top_srcdir)/lib/protocols/HttpProtocol.h \
#    $(top_srcdir)/lib/protocols/HttpProtocolImpl.h \
#    $(top_srcdir)/lib/protocols/HttpProtocol.cpp
#HttpProtocol_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
#HttpProtocol_unittest_LDADD = \
#    gtest/lib/libgtest_main.la \
#    $(top_srcdir)/lib/utility/libutility.la \
#    ${LIBCURL_LIBS} \
#    ${BOOST_LDFLAGS} \
#    ${BOOST_SIGNALS_LIB} \
#    $(GLIB_LIBS)

#TESTS += ProtocolFactory_unittest
#check_PROGRAMS += ProtocolFactory_unittest
#ProtocolFactory_unittest_SOURCES = \
#    ProtocolFactory_unittest.cpp \
#    $(top_srcdir)/lib/ProtocolFactory.h \
#    $(top_srcdir)/lib/ProtocolFactory.cpp
#ProtocolFactory_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
#ProtocolFactory_unittest_LDADD = \
#    gtest/lib/libgtest_main.la \
#    $(top_srcdir)/lib/protocols/libprotocols.la \
#    $(top_srcdir)/lib/utility/libutility.la \
#    ${LIBCURL_LIBS} \
#    ${BOOST_LDFLAGS} \
#    ${BOOST_SIGNALS_LIB} \
#    $(GLIB_LIBS)

#TESTS += DownloadManager_unittest
#check_PROGRAMS += DownloadManager_unittest
#DownloadManager_unittest_SOURCES = \
#    DownloadManager_unittest.cpp \
#    $(top_srcdir)/lib/ProtocolFactory.h \
#    $(top_srcdir)/lib/ProtocolFactory.cpp \
#    $(top_srcdir)/DownloadManager.h \
#    $(top_srcdir)/lib/DownloadManager.cpp
#DownloadManager_unittest_CPPFLAGS = ${LIBCURL_CPPFLAGS} ${BOOST_CPPFLAGS} $(GLIB_CFLAGS)
#DownloadManager_unittest_LDADD = \
#    gtest/lib/libgtest_main.la \
#    $(top_srcdir)/lib/protocols/libprotocols.la \
#    $(top_srcdir)/lib/utility/libutility.la \
#    ${LIBCURL_LIBS} \
#    ${BOOST_LDFLAGS} \
#    ${BOOST_SIGNALS_LIB} \
#    $(GLIB_LIBS)
