#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include "utility/BitMap.h"

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__GNUC__) && \
  !defined(__CYGWIN__) || defined(__MINGW32__)

#if !(defined(_WINSOCKAPI_) || defined(_WINSOCK_H))
/* The check above prevents the winsock2 inclusion if winsock.h already was
   included, since they can't co-exist without problems */
#include <winsock2.h>
#endif

#else

/* HP-UX systems version 9, 10 and 11 lack sys/select.h and so does oldish
   libc5-based Linux systems. Only include it on system that are known to
   require it! */
#if defined(_AIX) || defined(__NOVELL_LIBC__) || defined(__NetBSD__) || \
    defined(__minix) || defined(__SYMBIAN32__) || defined(__INTEGRITY)
#include <sys/select.h>
#endif

#ifndef _WIN32_WCE
#include <sys/socket.h>
#endif

#if !defined(WIN32) && !defined(__WATCOMC__)
#include <sys/time.h>
#endif

#include <sys/types.h>

#endif

typedef int TaskId;

enum SpeedUnit
{
    BYTES,
    KILOBYTES,
    MEGABYTES,
    GIGABYTES,
};

struct TaskInfo
{
    const char *url;
    const char *outputPath;
    const char *outputName;

    // Options is a string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    char *options;

    // if (total_size == 0) && (finish_size > 0) means not start download.
    size_t totalSize;
    size_t finishSize;

    BitMap bitMap;
    const char *comment;

    // no need save below
    size_t totalConnections;
    size_t validConnections;

    double downloadSpeed;
    SpeedUnit downloadUnit;
    double uploadSpeed;
    SpeedUnit uploadUnit;

    TaskInfo()
        : url(NULL),
          outputPath(NULL),
          outputName(NULL),
          options(NULL),
          totalSize(0),
          finishSize(1),
          comment(NULL),
          totalConnections(0),
          validConnections(0),
          downloadSpeed(0),
          downloadUnit(BYTES),
          uploadSpeed(0),
          uploadUnit(BYTES)
        {}
};

#endif
