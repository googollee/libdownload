#include "DownloadManager.h"

#include <vector>
#include <string>

struct TaskData
{
    TaskInfo *info;
    std::string data;
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

    TaskData data;
    data.info = info;
    d->tasks.push_back(data);

    return Task(info);
}

bool DownloadManager::isTaskExist(Task task)
{
    LOG(0, "enter DownloadManager::isTaskExist, task = %p\n", task.info_);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info == task.info_)
            return true;
    }

    return false;
}

bool DownloadManager::removeTask(Task task)
{
    LOG(0, "enter DownloadManager::removeTask, task = %p\n", task.info_);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info == task.info_)
        {
            it->info->protocol->removeTask(task.info_);
            delete it->info;
            return true;
        }
    }

    return false;
}

bool DownloadManager::startTask(Task task)
{
    LOG(0, "enter DownloadManager::startTask, task = %p\n", task.info_);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info == task.info_)
        {
            if ( (it->info->state == TASK_FINISH) || (it->info->state == TASK_DOWNLOAD) )
            {
                LOG(0, "task %d state is %d, can't start\n", task.id(), it->info->state);
                return false;
            }

            if (it->data.length() == 0)
                it->info->protocol->addTask(it->info);
            else
            {
                std::istringstream in(it->data);
                it->info->protocol->loadTask(it->info, in);
            }
            it->info->state = TASK_DOWNLOAD;

            return true;
        }
    }

    return false;
}

bool DownloadManager::stopTask(Task task)
{
    LOG(0, "enter DownloadManager::stopTask, task = %p\n", task.info_);

    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        if (it->info == task.info_)
        {
            if (it->info->state != TASK_DOWNLOAD)
            {
                LOG(0, "task %d state is %d, can't stop\n", task.id(), it->info->state);
                return false;
            }

            it->data.clear();
            std::ostringstream out(it->data);
            it->info->protocol->saveTask(task.info_, out);
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
    for (ProtocolFactory::Protocols::iterator it = d->factory->protocols_.begin();
         it != d->factory->protocols_.end();
         ++it)
    {
        ret += (*it)->perform();
    }

    return ret;
}

