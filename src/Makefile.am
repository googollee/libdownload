SUBDIRS = utility protocols

lib_LTLIBRARIES = libdownload.la

libdownload_la_LIBADD = \
    $(top_srcdir)/src/protocols/libprotocols.la \
    $(top_srcdir)/src/utility/libutility.la \
    ${LIBCURL_LIBS} \
    ${BOOST_LDFLAGS} \
    ${BOOST_SIGNALS_LIB} \
    $(GLIB_LIBS)

libdownload_la_CPPFLAGS = ${BOOST_CPPFLAGS}
libdownload_la_SOURCES = \
    Global.h \
    DownloadManager.cpp \
    DownloadManager.h \
    ProtocolFactory.cpp \
    ProtocolFactory.h
