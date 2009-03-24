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
    typedef void DownloadFinishCallback(TaskInfo *info);
    /**
     * \brief Callback when task download finish.
     *
     * When task download finish, call for noticing manager.
     * \param info The finished task's info.
     *
     */
    boost::signal<DownloadFinishCallback> downloadFinish;

    typedef void UploadFinishCallback(TaskInfo *info);
    /**
     * \brief Callback when task upload finish.
     *
     * When task Upload finish, call for noticing manager.
     * \param info The finished task's info.
     */
    boost::signal<UploadFinishCallback> uploadFinish;

    typedef void TaskErrorCallback(TaskInfo *info, int error);
    /**
     * \brief Callback when task need log something.
     *
     * Log informations for task.
     * For example: connect, redirect, or fail information. No need sufix with '\n'.
     * \param info The task's info which need be logged.
     * \param log  Log text.
     */
    boost::signal<TaskErrorCallback> taskError;

    typedef void TaskLogCallback(TaskInfo *info, const char *log);
    /**
     * \brief Callback when task meet error.
     *
     * When task meet error, call for noticing manager.
     * \param info  The error task's info.
     * \param error The error number.
     */
    boost::signal<TaskLogCallback> taskLog;

    typedef void ProtocolLogCallback(ProtocolBase *p, const char *log);
    /**
     * \brief Callback when protocol need log something.
     *
     * Log informations for protocol plugin.
     * For example: init, read configure, or fail information. No need sufix with '\n'.
     * \param p   The pointer to protocol instance
     * \param log Log text.
     */
    boost::signal<ProtocolLogCallback> protocolLog;

    ProtocolBase();
    virtual ~ProtocolBase();

    /**
     * \brief The name of protocol.
     *
     * Return protocol name for display, in utf8 codec.
     * It's better to return a statice string, which won't change in protocol instance life circle.
     *
     * \return The name text in utf8.
     */
    virtual const char* name() = 0;

    /**
     * \brief Check whether uri can be handled with this protocol.
     *
     * Check whether uri can be handled by this protocol.
     *
     * \param uri The uri need check. It should be in utf8 codec
     * \return The check result.
     */
    virtual bool canProcess    (const char *uri) = 0;

    /**
     * \brief Get the options detail description.
     *
     * Get the options detail description in XML. The XML format is like:
     * \code
     * <OptionName>
     *   <title>OptionTitleForShort</title>
     *   <desc>Detail description of this option</desc>
     *   <format>value format of this option</format>
     * </OptionName>
     * \endcode
     *
     * \return The XML text in utf8.
     * \see getTaskOptions()
     * \see getAllOptions()
     * \todo Doesn't sure the format of option format now, should use some thing easy convert to html.
     */
    virtual const char* getOptionsDetail() = 0;

    /**
     * \brief Get the options when adding uri as task.
     *
     * Some time user want control the download task, and need use this API get the options of this protocol task.
     * The options is a XML text. The detail of option tag can be found with getOptionsDetail().
     * For example, when download a bt feed, maybe only download one file in feed but not all, then caller can get the file list from getTaskOptions() and set the download file in options when call addTask().
     *
     * Return text like below:
     * \code
     * <OptionName1>value1</OptionName1>
     * <OptionName2>value2</OptionName2>
     * \endcode
     *
     * \param uri task uri in utf8 codec.
     * \return options text in XML.
     * \see getOptionsDetail()
     * \see addTask()
     */
    virtual const char* getTaskOptions(const char *uri) = 0;

    /**
     * \brief Load protocol's option.
     *
     * Load protocols option from istream. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \param in The input stream.
     * \see saveOptions()
     */
    virtual void loadOptions(std::istream &in) = 0;

    /**
     * \brief Save protocol's option.
     *
     * Save protocols option to ostream. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \param out The output stream.
     * \see loadOptions()
     */
    virtual void saveOptions(std::ostream &out) = 0;

    /**
     * \brief Return the all options for protocol.
     * Return the all options for protocol. For example, max opened socket.
     *
     * Return text like below:
     * \code
     * <OptionName1>value1</OptionName1>
     * <OptionName2>value2</OptionName2>
     * \endcode
     *
     * \return The XML text of all options.
     * \see saveOptions()
     * \see control()
     * \todo Maybe can merge with saveOptions() or control().
     */
    virtual const char* getAllOptions() = 0;

    /**
     * \brief Control the protocol options.
     *
     * Control the protocol property.
     * cmd format, e.g.:
     * f = CF_SET, key = "SessionNumber", value point to a size_t variable.
     * HttpProtocol will set the default session number per task as value.
     *
     * \notice No standard command now.
     *
     * \param f Control flag, set or get.
     * \param key The string indicate which option need be controlled.
     * \param value Point to the variable.
     */
    virtual void control(ControlFlag f, const char* key, void *value) = 0;

    /**
     * \brief Add a task.
     *
     * TaskInfo is controled by manager, can modify in protocol
     * If info->processData doesn't empty, need resume task from processData.
     *
     * \param info The task info.
     */
    virtual void addTask      (TaskInfo *info) = 0;

    /**
     * \brief Flush the process data into info->processData
     *
     * \param info The task info.
     */
    virtual void flushTask    (TaskInfo *info) = 0;

    /**
     * \brief Stop and remove a task.
     *
     * \param info The task info.
     */
    virtual void removeTask   (TaskInfo *info) = 0;

    /**
     * \brief Check if a task is processing.
     *
     * \param info The task info.
     * \return The check result.
     */
    virtual bool hasTask      (TaskInfo *info) = 0;

    /**
     * \brief Control the task options.
     *
     * Control the task property.
     * cmd format, e.g.:
     * f = CF_SET, key = "SessionNumber", value point to a size_t variable.
     * HttpProtocol will set the default session number per task as value.
     *
     * \notice No standard command now.
     *
     * \param f Control flag, set or get.
     * \param key The string indicate which option need be controlled.
     * \param value Point to the variable.
     */
    virtual void controlTask  (TaskInfo *info, ControlFlag f, const char* key, void *value) = 0;

    /**
     * \brief Extracts file descriptor information.
     *
     * This function extracts file descriptor information. The application can use these to select() on, but be sure to FD_ZERO them before calling this function.
     *
     * \param read_fd_set Pointer to save read fd set
     * \param write_fd_set Pointer to save write fd set
     * \param exc_fd_set Pointer to save except fd set
     * \param max_fd Pointer to save the max fd.
     */
    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int    *max_fd) = 0;

    /**
     * \brief Perform download/upload.
     *
     * \return The processing tasks number.
     */
    virtual int perform() = 0;

    /**
     * \brief Return the string of error.
     *
     * \param error Error number.
     * \return The text of error.
     */
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
