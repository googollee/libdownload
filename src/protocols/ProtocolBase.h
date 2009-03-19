#ifndef PROTOCOL_BASE_HEADER
#define PROTOCOL_BASE_HEADER

#include "Global.h"
#include "utility/Utility.h"

#include <boost/signals.hpp>
#include <istream>
#include <ostream>

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

enum ControlFlag
{
    CF_SET,
    CF_GET,
};

class ProtocolBase : private Noncopiable
{
public:
    // when task download finish, call for noticing manager.
    typedef void DownloadFinishCallback(TaskInfo *info);

    // when task Upload finish, call for noticing manager.
    typedef void UploadFinishCallback(TaskInfo *info);

    // log informations for task. For example: connect, redirect, or fail information. No need sufix with '\n'.
    typedef void TaskLogCallback(TaskInfo *info, const char *log);

    // when task meet error, call for noticing manager.
    typedef void TaskErrorCallback(TaskInfo *info, int error);

    // log informations for protocol plugin. For example: init, read configure, or fail information. No need sufix with '\n'.
    typedef void ProtocolLogCallback(ProtocolBase *p, const char *log);

    boost::signal<DownloadFinishCallback> downloadFinish;
    boost::signal<UploadFinishCallback> uploadFinish;
    boost::signal<TaskErrorCallback> taskError;
    boost::signal<TaskLogCallback> taskLog;
    boost::signal<ProtocolLogCallback> protocolLog;

    ProtocolBase();
    virtual ~ProtocolBase();

    // Return protocol name for display, in utf8 codec
    virtual const char* name() = 0;

    // Check whether uri can be processed by this protocol
    // uri should in utf8 codec
    virtual bool        canProcess    (const char *uri) = 0;
    // Get the options when adding uri as task.
    virtual const char* getTaskOptions(const char *uri) = 0;

    // Options
    // opt is a utf8 text like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    // load must be end before "</data>"
    virtual void loadOptions(std::istream &in) = 0;
    virtual void saveOptions(std::ostream &out) = 0;

    // Return the all options for protocol. For example, max opened socket.
    // return format must be below in utf8 codec:
    // <key1 default="default value" help="help of key1">value</key1>
    // <key2 default="default value" help="help of key2" /> <!-- short for only default value -->
    virtual const char* getAllOptions() = 0;

    // Control the protocol property.
    // cmd format, e.g.:
    // f = CF_SET, key = "SessionNumber", value point to a size_t variable.
    // HttpProtocol will save session number per task in value.
    //
    // no standard command now.
    virtual void control(ControlFlag f, const char* key, void *value) = 0;

    // Task control
    // TaskInfo is controled by manager, can modify in protocol
    // If info->processData doesn't empty, need do resume.
    virtual void addTask      (TaskInfo *info) = 0;
    virtual void removeTask   (TaskInfo *info) = 0;
    virtual bool hasTask      (TaskInfo *info) = 0;
    virtual void controlTask  (TaskInfo *info, ControlFlag f, const char* key, void *value) = 0;

    // Save and load
    // load must be end before "</data>"
    virtual void loadTask(TaskInfo *info, std::istream &in) = 0;
    virtual void saveTask(TaskInfo *info, std::ostream &out) = 0;

    // Perform download
    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int    *max_fd) = 0;

    // return the downloading items number
    // downloaded and uploaded speed unit is byte.
    virtual int perform() = 0;

    // return the string of error
    virtual const char *strerror(int error) = 0;

private:
    ProtocolBase(const ProtocolBase &);
    bool operator=(const ProtocolBase &);
};

inline ProtocolBase::ProtocolBase()
{}

inline ProtocolBase::~ProtocolBase()
{}

#endif
