#ifndef DOWNLOAD_MANAGER_HEADER
#define DOWNLOAD_MANAGER_HEADER

#include "Global.h"
#include "protocols/ProtocolBase.h"
#include "utility/Utility.h"

class Task
{
public:
    TaskId      id()           { return info->id; }
    const char* uri()          { return info->uri.c_str(); }
    const char* outputPath()   { return info->outputPath.c_str(); }
    const char* outputname()   { return info->outputName.c_str(); }
    const char* options()      { return info->options.c_str(); }
    const char* comment()      { return info->comment.c_str(); }
    size_t      totalSize()    { return info->totalSize; }
    size_t      downloadSize() { return info->downloadSize; }
    size_t      uploadSize()   { return info->uploadSize; }
    BitMap      validMap()     { return info->validMap; }
    BitMap      downloadMap()  { return info->downloadMap; }
    int         totalSource()  { return info->totalSource; }
    int         validSource()  { return info->validSource; }
    TaskState   state()        { return info->state; }

    bool operator== (const Task &rhs)
        { return info_ == rhs.info_; }
    bool operator!= (const Task &rhs)
        { return !operator==(rhs); }

private:
    friend class DownloadManager;

    Task(TaskInfo *info);

    TaskInfo *info_;
};

class DownloadManager : private Noncopiable
{
public:
    DownloadManager();
    ~DownloadManager();

    const char* getTaskOptions(const char *url);
    Task addTask(const char *url,
                 const char *outputPath,
                 const char *outputName,
                 const char *options,
                 const char *comment);
    bool isTaskExist(const TaskId id);
    bool removeTask(const TaskId id);

    bool startTask(const TaskId id);
    bool stopTask(const TaskId id);

    int perform(size_t *download, size_t *upload);
};

#endif
