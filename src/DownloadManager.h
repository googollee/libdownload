#ifndef DOWNLOAD_MANAGER_HEADER
#define DOWNLOAD_MANAGER_HEADER

#include "Global.h"
#include "ProtocolFactory.h"
#include "protocols/ProtocolBase.h"
#include "utility/Utility.h"

#include <boost/signals.hpp>

#include <memory>
#include <istream>
#include <ostream>

class Task
{
public:
    int         id()           { return (int)info_; }
    const char* uri()          { return info_->uri.c_str(); }
    const char* outputPath()   { return info_->outputPath.c_str(); }
    const char* outputname()   { return info_->outputName.c_str(); }
    const char* options()      { return info_->options.c_str(); }
    const char* comment()      { return info_->comment.c_str(); }
    size_t      totalSize()    { return info_->totalSize; }
    size_t      downloadSize() { return info_->downloadSize; }
    size_t      uploadSize()   { return info_->uploadSize; }
    BitMap      validMap()     { return info_->validMap; }
    BitMap      downloadMap()  { return info_->downloadMap; }
    int         totalSource()  { return info_->totalSource; }
    int         validSource()  { return info_->validSource; }
    TaskState   state()        { return info_->state; }

    bool operator== (const Task &rhs)
        { return info_ == rhs.info_; }
    bool operator!= (const Task &rhs)
        { return !operator==(rhs); }

private:
    friend class DownloadManager;

    Task(TaskInfo *info)
        : info_(info)
        {}

    TaskInfo *info_;
};

struct DownloadManagerData;

/**
 * Normal usage:
 * ProtocolFactory *factory = new ProtocolFactory();
 * factory->addProtocol(...);
 *
 * DownloadManager manager(auto_prt<ProtocolFactory>(factory));
 * string taskOptions = manager.getTaskOptions(uri);
 * ...determine the task options...
 * Task task = manager.addTask(uri, outputPath, outputName, taskOptions.c_str(), comment);
 * manager.startTask(task);
 *
 * while (manager.perform() > 0) {}
 *
 * manager.removeTask(task.id());
 *
 * download end.
 */
class DownloadManager : private Noncopiable
{
public:
    typedef void TaskStateChangeCallback(Task task, TaskState oldState, TaskState newState);
    boost::signal<TaskStateChangeCallback> taskStateChange;

    explicit DownloadManager(std::auto_ptr<ProtocolFactory> factory);
    ~DownloadManager();

    const char* strerror(int err);

    bool        canDownload   (const char *uri);
    const char* getTaskOptions(const char *uri);

    Task addTask(const char *uri,
                 const char *outputPath,
                 const char *outputName,
                 const char *options,
                 const char *comment);
    bool isTaskExist(Task task);
    bool removeTask (Task task);
    bool startTask  (Task task);
    bool stopTask   (Task task);

    void load(std::istream &in);
    void save(std::ostream &out);

    int perform();

private:
    std::auto_ptr<DownloadManagerData> d;
};

#endif
