#ifndef INCLUDES_HEAD
#define INCLUDES_HEAD

#include "Utility.h"
#include "DownloadException.h"
#include "SingleCurlInitHelper.h"

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__GNUC__) && \
  !defined(__CYGWIN__) || defined(__MINGW32__)

#if !(defined(_WINSOCKAPI_) || defined(_WINSOCK_H))
/* The check above prevents the winsock2 inclusion if winsock.h already was
   included, since they can't co-exist without problems */
#include <winsock2.h>
#endif // !(defined(_WINSOCKAPI_) || defined(_WINSOCK_H))

#else

/* HP-UX systems version 9, 10 and 11 lack sys/select.h and so does oldish
   libc5-based Linux systems. Only include it on system that are known to
   require it! */
#if defined(_AIX) || defined(__NOVELL_LIBC__) || defined(__NetBSD__) || \
    defined(__minix) || defined(__SYMBIAN32__) || defined(__INTEGRITY)
#include <sys/select.h>
#endif // defined(_AIX) || defined(__NOVELL_LIBC__) ..  defined(__INTEGRITY)

#ifndef _WIN32_WCE
#include <sys/socket.h>
#endif // !_WIN32_WCE

#if !defined(WIN32) && !defined(__WATCOMC__)
#include <sys/time.h>
#endif // !defined(WIN32) && !defined(__WATCOMC__)

#include <sys/types.h>

#endif // defined(WIN32) && !defined(_WIN32_WCE) && .. || defined(__MINGW32__)

#include <boost/signals.hpp>

#endif
