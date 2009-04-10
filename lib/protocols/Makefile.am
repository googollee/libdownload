noinst_LTLIBRARIES = libprotocols.la

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/lib/

libprotocols_la_SOURCES = \
    ProtocolBase.h \
    HttpProtocol.h \
    HttpProtocolImpl.h \
    HttpProtocol.cpp

libprotocols_la_CPPFLAGS = ${BOOST_CPPFLAGS} ${LIBCURL_CFLAGS}
libprotocols_la_LDFLAGS = ${LIBCURL_LIBS}
