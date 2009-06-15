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
    /**
     * \brief Callback when protocol need log something.
     *
     * Log informations for protocol plugin.
     * For example: init, read configure, or fail information. No need sufix with '\n'.
     * \param p   The pointer to protocol instance
     * \param log Log text.
     */
    boost::signal<void (ProtocolBase *p, const char *log)> protocolLog;

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
     * \brief Get the options detail description.
     *
     * Get the options detail description in XML. The XML format is like:
     * \code
     * <OptionName>
     *   <title>OptionTitleForShort</title>
     *   <desc>Detail description of this option</desc>
     *   <type>value format of this option</type>
     *   <pattern>value pattern of this option, in regex</pattern>
     * </OptionName>
     * \endcode
     * \notice Values of type and pattern follow XML Schema special.
     *
     * \return The XML text in utf8.
     * \see getTaskOptions()
     * \see getAllOptions()
     */
    virtual const char* getOptionsDetail() = 0;

    /**
     * \brief Get protocol's option.
     *
     * Get protocols options. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \return The options.
     * \see saveOptions()
     */
    virtual const char* getOptions() = 0;

    /**
     * \brief Set protocol's option.
     *
     * Set protocols option. It used to resume the protocol status when relaunch.
     * It's different like the task options which can be different in different task. It's the global options of protocol or default value of task options, shared with all tasks.
     *
     * \param options The options.
     * \see loadOptions()
     */
    virtual void setOptions(const char *options) = 0;

    /**
     * \brief Check whether uri can be handled with this protocol.
     *
     * Check whether uri can be handled by this protocol.
     *
     * \param uri The uri need check. It should be in utf8 codec
     * \return The check result.
     */
    virtual bool canProcess(const char *uri) = 0;

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
     * \brief Add a task.
     *
     * TaskInfo is controled by manager, can modify in protocol
     * If info->processData doesn't empty, need resume task from processData.
     *
     * \param info The task info.
     */
    virtual void addTask(TaskInfo *info) = 0;

    /**
     * \brief Stop and remove a task.
     *
     * \param info The task info.
     */
    virtual void removeTask(TaskInfo *info) = 0;

    /**
     * \brief Check if a task is processing.
     *
     * \param info The task info.
     * \return The check result.
     */
    virtual bool hasTask(TaskInfo *info) = 0;

    /**
     * \brief Flush the process data into info->processData
     *
     * \param info The task info.
     */
    virtual void flushTask(TaskInfo *info) = 0;

    /**
     * \brief Get the task state.
     *
     * Get the task detail state, maybe the session position, or percentage of each session.
     *
     * \param info The task info.
     * \return The state detail of task.
     */
    virtual const char* getTaskState(TaskInfo *info) = 0;

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
     * \brief Perform download.
     *
     * \return The processing tasks number.
     */
    virtual int performDownload(size_t *size) = 0;

    /**
     * \brief Perform upload.
     *
     * \return The processing tasks number.
     */
    virtual int performUpload(size_t *size) = 0;

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
