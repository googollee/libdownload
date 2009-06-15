#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <utility/BitMap.h>

#include <boost/signals.hpp>

#include <string>

enum TaskState
{
    TASK_WAIT,
    TASK_DOWNLOAD,
    TASK_FINISH,
    TASK_ERROR,
};

class ProtocolBase;

struct TaskInfo
{
    /**
     * \brief Callback when task download finish.
     *
     * When task download finish, call for noticing manager.
     * \param info The finished task's info.
     *
     */
    boost::signal<void (TaskInfo *info)> downloadFinish;

    /**
     * \brief Callback when task upload finish.
     *
     * When task Upload finish, call for noticing manager.
     * \param info The finished task's info.
     */
    boost::signal<void (TaskInfo *info)> uploadFinish;

    /**
     * \brief Callback when task need log something.
     *
     * Log informations for task.
     * For example: connect, redirect, or fail information. No need sufix with '\n'.
     * \param info The task's info which need be logged.
     * \param log  Log text.
     */
    boost::signal<void (ProtocolBase *p, TaskInfo *info, int error)> taskError;

    /**
     * \brief Callback when task meet error.
     *
     * When task meet error, call for noticing manager.
     * \param info  The error task's info.
     * \param error The error number.
     */
    boost::signal<void (TaskInfo *info, const char *log)> taskLog;

    std::string uri;
    std::string outputPath;
    std::string outputName;
    // Options is a string like:
    // <key1>value1</key1>
    // <key2>value2</key2>
    //...
    std::string options;
    std::string comment;
    std::string mimeType;

    std::string processData;

    // no need save below
    size_t totalSize;
    size_t downloadSize;

    size_t uploadSize;

    BitMap validMap;
    BitMap downloadMap;

    int totalSource;
    int validSource;

    TaskState state;
    ProtocolBase *protocol;

    TaskInfo()
        : totalSize(0),
          downloadSize(0),
          uploadSize(0),
          totalSource(0),
          validSource(0),
          state(TASK_WAIT),
          protocol(NULL)
        {}
};

#endif
