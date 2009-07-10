#ifndef TASK_HEADER
#define TASK_HEADER

#include "includes.h"

#include <vector>

enum TaskState
{
    TASK_WAIT,
    TASK_WORK,
    TASK_FINISH,
    TASK_ERROR,
};

class ProtocolBase;

template<typename File>
class Task
{
public:
    /**
     * \brief Callback when task download finish.
     *
     * When task download finish, call for noticing manager.
     * \param info The finished task's info.
     *
     */
    static boost::signal<void (Task *task)> downloadFinishSignal;

    /**
     * \brief Callback when task upload finish.
     *
     * When task Upload finish, call for noticing manager.
     * \param info The finished task's info.
     */
    static boost::signal<void (Task *task)> uploadFinishSignal;

    /**
     * \brief Callback when task meet error.
     *
     * When task meet error, call for noticing manager.
     * \param info  The error task's info.
     * \param error The error number.
     */
    static boost::signal<void (Task *task, int error)> errorSignal;

    /**
     * \brief Callback when task need log something.
     *
     * Log informations for task.
     * For example: connect, redirect, or fail information. No need sufix with '\n'.
     * \param info The task's info which need be logged.
     * \param log  Log text.
     */
    static boost::signal<void (Task *task, const char *log)> logSignal;

    void downloadFinish()
        {
            downloadFinishSignal(this);
        }

    void uploadFinish()
        {
            uploadFinishSignal(this);
        }

    void error(int error)
        {
            taskErrorSignal(this, error);
        }

    void log(const char *log)
        {
            logSignal(this, log);
        }

    Task() {}
    virtual ~Task() {}

    virtual const char *getUri() = 0;
    virtual const char *getOutputDir() = 0;
    virtual const char *getOutputName() = 0;
    virtual const char *getOptions() = 0;
    virtual const char *getMimeType() = 0;
    virtual const char *getComment() = 0;
    virtual size_t      getTotalSize() = 0;
    virtual size_t      getDownloadSize() = 0;
    virtual size_t      getUploadSize() = 0;
    virtual int         getTotalSource() = 0;
    virtual int         getValidSource() = 0;
    virtual std::vector<bool> getValidBitmap() = 0;
    virtual std::vector<bool> getDownloadBitmap() = 0;
    virtual TaskState   getState() = 0;
    virtual ProtocolBase *getProtocol() = 0;

    virtual void getFdSet(fd_set *read, fd_set *write, fd_set *exc, int *max) = 0;
    virtual size_t performDownload() = 0;
    virtual size_t performUpload() = 0;

    virtual const char *strerror(int error) = 0;
};

#endif
