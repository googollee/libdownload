#include "DownloadManager.h"

#include <vector>
#include <string>
#include <algorithm>

typedef std::vector<TaskInfo*> Tasks;

struct DownloadManagerData
{
    ProtocolFactory *factory;
    Tasks tasks;

    DownloadManagerData(ProtocolFactory *f)
        : factory(f)
        {}
};

DownloadManager::DownloadManager(ProtocolFactory *factory)
    : d(new DownloadManagerData(factory))
{}

DownloadManager::~DownloadManager()
{}

enum DownloadManagerErr
{
    URI_NULL,
    OUTPUTPATH_NULL,
    NO_PROTOCOL,
    NO_TASK,
    LAST_ERROR = NO_TASK,
};

const char* DownloadManager::strerror(int err)
{
    static const char *errorString[] =
        {
            "uri should not empty.",          // URI_NULL
            "output path should not empty.",  // OUTPUTPATH_NULL
            "no protocol can process.",       // NO_PROTOCOL
            "no special task.",               // NO_TASK
        };

    if ( (URI_NULL <= err) && (err <= LAST_ERROR) )
        return errorString[err];

    return "unknown error.";
}

const char* DownloadManager::getTaskOptions(const char *uri)
{
    ProtocolBase *p = d->factory->getProtocol(uri);
    if (p == NULL)
        return NULL;

    return p->getTaskOptions(uri);
}

Task DownloadManager::addTask(const char *uri,
                              const char *outputPath,
                              const char *outputName,
                              const char *options,
                              const char *comment)
{
    LOG(0, "enter DownloadManager::addTask, uri = %s, outputPath = %s, outputName = %s, options = %s, comment = %s\n",
        (uri        == NULL) ? "NULL" : uri,
        (outputPath == NULL) ? "NULL" : outputPath,
        (outputName == NULL) ? "NULL" : outputName,
        (options    == NULL) ? "NULL" : options,
        (comment    == NULL) ? "NULL" : comment);

    if (uri == NULL)
    {
        LOG(0, "uri can't be NULL when add task.");
        DOWNLOADEXCEPTION(URI_NULL, "DownloadManager", strerror(URI_NULL));
    }
    if (outputPath == NULL)
    {
        LOG(0, "outputPath can't be NULL when add task.");
        DOWNLOADEXCEPTION(OUTPUTPATH_NULL, "DownloadManager", strerror(OUTPUTPATH_NULL));
    }

    ProtocolBase *p = d->factory->getProtocol(uri);
    if (p == NULL)
    {
        LOG(0, "don't know how to download %s", uri);
        DOWNLOADEXCEPTION(NO_PROTOCOL, "DownloadManager", strerror(NO_PROTOCOL));
    }

    TaskInfo *info = new TaskInfo;
    info->uri = uri;
    info->outputPath = outputPath;
    if (outputName != NULL) info->outputName = outputName;
    if (options    != NULL) info->options    = options;
    if (comment    != NULL) info->comment    = comment;
    info->protocol = p;
    info->state = TASK_WAIT;

    d->tasks.push_back(info);

    return Task(info);
}

bool DownloadManager::isTaskExist(Task task)
{
    LOG(0, "enter DownloadManager::isTaskExist, task = %p\n", task.info_);

    Tasks::iterator it = std::find(d->tasks.begin(), d->tasks.end(), task.info_);

    return it != d->tasks.end();
}

bool DownloadManager::removeTask(Task task)
{
    LOG(0, "enter DownloadManager::removeTask, task = %p\n", task.info_);

    if (std::find(d->tasks.begin(), d->tasks.end(), task.info_) != d->tasks.end())
    {
        task.info_->protocol->removeTask(task.info_);
        delete task.info_;

        return true;
    }

    return false;
}

bool DownloadManager::startTask(Task task)
{
    LOG(0, "enter DownloadManager::startTask, task = %p\n", task.info_);

    if (std::find(d->tasks.begin(), d->tasks.end(), task.info_) != d->tasks.end())
    {
        if ( (task.info_->state == TASK_FINISH) || (task.info_->state == TASK_DOWNLOAD) )
        {
            LOG(0, "task %p state is %d, can't start\n", task.id(), task.info_->state);
            return false;
        }

        task.info_->protocol->addTask(task.info_);
        task.info_->state = TASK_DOWNLOAD;

        return true;
    }

    return false;
}

bool DownloadManager::stopTask(Task task)
{
    LOG(0, "enter DownloadManager::stopTask, task = %p\n", task.info_);

    if (std::find(d->tasks.begin(), d->tasks.end(), task.info_) != d->tasks.end())
    {
        if (task.info_->state != TASK_DOWNLOAD)
        {
            LOG(0, "task %p state is %d, can't stop\n", task.id(), task.info_->state);
            return false;
        }

        task.info_->protocol->flushTask(task.info_);
        task.info_->state = TASK_WAIT;

        return true;
    }

    return false;
}

void DownloadManager::load(std::istream &in)
{
    LOG(0, "enter DownloadManager::load\n");
}

void DownloadManager::save(std::ostream &out)
{
    LOG(0, "enter DownloadManager::save\n");
}

int DownloadManager::perform(size_t *download, size_t *upload)
{
//    LOG(0, "enter DownloadManager::perform()\n");

    int ret = 0;
    size_t size = 0;
    for (ProtocolFactory::Protocols::iterator it = d->factory->protocols_.begin();
         it != d->factory->protocols_.end();
         ++it)
    {
        size = 0;
        ret += (*it)->performDownload(&size);
        download += size;
        size = 0;
        ret += (*it)->performUpload(&size);
        upload += size;
    }

    return ret;
}

