#ifndef TASK_HEADER
#define TASK_HEADER

#include <vector>

#include <boost/signals.hpp>

#include "utility/socket.h"

class ProtocolBase;

class TaskBase
{
public:
    enum TaskState
    {
        TASK_WAIT,
        TASK_DOWNLOAD,
        TASK_UPLOAD,
        TASK_FINISH,
        TASK_ERROR,
    };

    /**
     * \brief Callback when task download finish.
     *
     * When task download finish, call for noticing manager.
     * \param info The finished task's info.
     *
     */
    static boost::signal<void (TaskBase* task)> downloadFinishSignal;

    /**
     * \brief Callback when task upload finish.
     *
     * When task Upload finish, call for noticing manager.
     * \param info The finished task's info.
     */
    static boost::signal<void (TaskBase* task)> uploadFinishSignal;

    /**
     * \brief Callback when task meet error.
     *
     * When task meet error, call for noticing manager.
     * \param info  The error task's info.
     * \param error The error number.
     */
    static boost::signal<void (TaskBase* task, int error)> errorSignal;

    /**
     * \brief Callback when task need log something.
     *
     * Log informations for task.
     * For example: connect, redirect, or fail information. No need sufix with '\n'.
     * \param info The task's info which need be logged.
     * \param log  Log text.
     */
    static boost::signal<void (TaskBase* task, const char* log)> logSignal;

    void downloadFinish()
        {
            TaskBase::downloadFinishSignal(this);
        }

    void uploadFinish()
        {
            TaskBase::uploadFinishSignal(this);
        }

    void error(int error)
        {
            TaskBase::errorSignal(this, error);
        }

    void log(const char* log)
        {
            TaskBase::logSignal(this, log);
        }

    TaskBase() {}
    virtual ~TaskBase() {}

    virtual const char* uri() = 0;
    virtual const char* outputDir() = 0;
    virtual const char* outputName() = 0;
    virtual const char* options() = 0;
    virtual const char* mimeType() = 0;
    virtual const char* comment() = 0;
    virtual const char* notice() = 0;
    virtual size_t      totalSize() = 0;
    virtual size_t      downloadSize() = 0;
    virtual size_t      uploadSize() = 0;
    virtual int         totalSource() = 0;
    virtual int         validSource() = 0;
    virtual std::vector<bool> validBitmap() = 0;
    virtual std::vector<bool> downloadBitmap() = 0;
    virtual TaskState   state() = 0;
    virtual ProtocolBase* protocol() = 0;

    virtual bool start() = 0;
    virtual bool stop() = 0;

    virtual bool fdSet(fd_set* read, fd_set* write, fd_set* exc, int* max) = 0;
    virtual size_t performDownload() = 0;
    virtual size_t performUpload() = 0;

    virtual int error() = 0;
    virtual const char* strerror(int error) = 0;
};

#endif
