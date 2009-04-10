SUBDIRS = utility protocols

lib_LTLIBRARIES = libdownload.la

libdownload_la_LIBADD = \
    $(top_srcdir)/lib/protocols/libprotocols.la \
    $(top_srcdir)/lib/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)

libdownload_la_CPPFLAGS = ${BOOST_CPPFLAGS}
libdownload_la_LDFLAGS = -no-undefined -version-info 0:0:0

libdownload_la_SOURCES = \
    Global.h \
    DownloadManager.cpp \
    DownloadManager.h \
    ProtocolFactory.cpp \
    ProtocolFactory.h

libdownloadincludedir=$(includedir)/libdownload
libdownloadinclude_HEADERS = \
    Global.h \
    DownloadManager.h \
    utility/BitMap.h \
    utility/Utility.h \
    utility/SimpleXmlParser.h \
    utility/File.h \
    utility/SingleCurlHelper.h \
    protocols/ProtocolBase.h \
    protocols/HttpProtocol.h \
    plugins/PrePluginBase.h \
    plugins/PostPluginBase.h
