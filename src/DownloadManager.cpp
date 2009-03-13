#include "DownloadManager.h"

#include <vector>
#include <string>

struct TaskData
{
    TaskInfo *info;
    std::string data
};

typedef std::vector<TaskData> Tasks;

struct DownloadManagerData
{
    std::auto_ptr<ProtocolFactory> factory;
    Tasks tasks;

    DownloadManagerData(std::auto_ptr<ProtocolFactory> f)
        : factory(f)
        {}
};

DownloadManager::DownloadManager(std::auto_ptr<ProtocolFactory> factory)
    : d(new DownloadManagerData(factory))
{}

DownloadManager::~DownloadManager()
{}

const char* DownloadManager::getTaskOptions(const char *uri)
{
    ProtocolBase *p = f->getProtocol(uri);
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

    ProtocolBase *p = f->getProtocol(uri);
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
    info->p = p;
    info->state = TASK_WAIT;

    TaskData data;
    data.info = info;
    d->tasks.push_back(data);

    return Task(info);
}

Task DownloadManager::getTask(const TaskId id)
{
    LOG(0, "enter DownloadManager::getTask, id = %d\n", id);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info->id == id)
            return Task(it->info);
    }

    LOG(0, "can't find task with id = %d", id);
    DOWNLOADEXCEPTION(NO_TASK, "DownloadManager", strerror(NO_TASK));
}

bool DownloadManager::isTaskExist(const TaskId id)
{
    LOG(0, "enter DownloadManager::isTaskExist, id = %d\n", id);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info->id == id)
            return true;
    }

    return false;
}

bool DownloadManager::removeTask(const TaskId id)
{
    LOG(0, "enter DownloadManager::removeTask, id = %d\n", id);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info->id == id)
        {
            it->info->protocol->removeTask(id);
            delete it->info;
            return true;
        }
    }

    return false;
}

bool DownloadManager::startTask(const TaskId id)
{
    LOG(0, "enter DownloadManager::startTask, id = %d\n", id);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info->id == id)
        {
            if ( (it->info->state == TASK_FINISH) || (it->info->state == TASK_DOWNLOAD) )
            {
                LOG(0, "task %d state is %d, can't start\n", id, it->info->state);
                return false;
            }

            if (it->data.length() == 0)
                it->info->protocol->addTask(it->info);
            else
                it->info->protocol->loadTask(it->info, std::istringstream(it->data));
            it->info->state = TASK_DOWNLOAD;

            return true;
        }
    }

    return false;
}

bool DownloadManager::stopTask(const TaskId id)
{
    LOG(0, "enter DownloadManager::stopTask, id = %d\n", id);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info->id == id)
        {
            if (it->info->state != TASK_DOWNLOAD)
            {
                LOG(0, "task %d state is %d, can't stop\n", id, it->info->state);
                return false;
            }

            it->data.clear();
            it->info->protocol->saveTask(id, std::ostringstream(it->data));
            it->info->state = TASK_WAIT;

            return true;
        }
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

int DownloadManager::perform()
{
    LOG(0, "enter DownloadManager::perform, downlaod = %p, upload = %p\n", download, upload);

    int ret = 0;
    for (ProtocolFactory::Protocols::iterator it = d->factory.protocols_.begin();
         it != d->factory.protocols_.end();
         ++it)
    {
        ret += it->perform();
    }

    return ret;
}

