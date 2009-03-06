/**
 * HttpProtocol Structure Map:
 *
 *                  +----------------------+
 *                  |   HttpProtocolData   |
 *                  +----------------------+
 *                  |CURLM *handle         |
 *          +-------|int running           |
 *          |       +----------------------+
 *          |                 <>
 *          |                  | 0..n
 *          |                  |
 *          |                  |
 *          |         +------------------+           +--------------+
 *          +-------<>|     HttpTask     |<>---------|TaskInfo *info|
 * HttpProtocolData *d+------------------+           +--------------+
 *                    |File file         |
 *                    |HttpConfigure conf|
 *            +-------|SessionState state|
 *            |       +------------------+
 *            |               <>
 *            |                | 1..n
 *            |                |
 *            |                |
 * HttpTask *t|       +------------------+
 *            +-----<>|     Session      |
 *                    +------------------+
 *             +------|size_t pos        |--+
 *             |      |size_t length     |  |
 *             |      +------------------+  |
 *             |              <>            |
 *             |               | 1          |
 *             |               |            |
 *             |               |            |
 *             |        +-------------+     |
 *             +------<>|CURL *handle |<>---+ WRITEFUNCTION_DATA
 *    PRIVATE_DATA      +-------------+
 */

#include "HttpProtocol.h"
#include "HttpProtocolImpl.h"

#include <utility/File.h>
#include <utility/DownloadException.h>
#include <utility/SimpleXmlParser.h>

#include <curl/curl.h>

#include <boost/format.hpp>

#include <cstring>
#include <string>
#include <algorithm>
#include <sstream>

#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
            throw DOWNLOADEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

enum HttpError
{
    CURL_BAD_ALLOC,
    CURLM_BAD_ALLOC,
    NULL_INFO,
    BAD_FILE_LENGTH,
    FAIL_OPEN_FILE,
    TASK_EXIST,
    TASK_NOT_EXIST,
};

size_t writeCallback(void *buffer, size_t size, size_t nmemb, HttpSession *ses)
{
    HttpTask *task = ses->t;

    if (task->state == HT_PREPARE)
    {
        task->d->initTask(task);
    }

    const size_t totalSize = size * nmemb;

    size_t shouldWrite = totalSize;
    if (ses->length > 0)
    {
        // got the file size from server.
        if (totalSize > size_t(ses->length))
            shouldWrite = ses->length;
        task->file.seek(ses->pos, filesystem::FromBegin);
    }

    const size_t writed = task->file.write(buffer, shouldWrite);

    task->info->downloadSize += writed;

    if (ses->length > 0)
    {
        task->info->downloadMap.setRangeByLength(ses->pos, ses->pos + writed, true);
        ses->length -= writed;
        ses->pos += writed;

        if (ses->length == 0)
        {
            task->d->finishSessions.push_back(ses);
            CURLcode rete = curl_easy_pause(ses->handle, CURLPAUSE_ALL);
            CHECK_CURLE(rete);
        }
    }

    return (writed == shouldWrite) ? totalSize : writed; // trick to avoid curl easy fail.
}

void getFileName(CURL *handle, HttpTask *task)
{
    const char *p = task->info->uri + strlen(task->info->uri);
    bool findNonNameChar = false;
    do
    {
        --p;
        switch (*p)
        {
        case '*':
        case '|':
        case '\\':
        case ':':
        case '"':
        case '<':
        case '>':
        case '?':
        case '/':
            findNonNameChar = true;
            break;
        }
    } while ( !findNonNameChar && (p >= task->info->uri) );
    ++p;

    if (task->info->outputName != NULL)
        delete [] task->info->outputName;

    task->info->outputName = strdup(p);
}

class HttpConfXmlParser : public SimpleXmlParser
{
private:
    void text(const char *text,
              size_t textLen)
        {
            printf("in parser::text, len = %lu, text = %s\n", textLen, text);
            while ( (*text == ' ') || (*text == '\n') )
            {
                ++text;
                --textLen;
                if (*text == '\0')
                    break;
            }
            while ( (text[textLen-1]  == ' ') || (text[textLen-1] == '\n') )
            {
                --textLen;
                if (textLen == 0)
                    break;
            }

            std::string data(text, textLen);
            if (strcmp(getElement(), "SessionNumber") == 0)
            {
                task_->conf.sessionNumber = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "MinSessionBlocks") == 0)
            {
                task_->conf.minSessionBlocks = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "BytesPerBlock") == 0)
            {
                task_->conf.bytesPerBlock = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "TotalSize") == 0)
            {
                task_->info->totalSize = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "BitMap") == 0)
            {
                task_->info->downloadMap = BitMap(task_->info->totalSize, task_->conf.bytesPerBlock);
                size_t finishSize = 0;
                for (int i=0, n=data.length()-1; i<n; ++i)
                {
                    if (data[i] == '1')
                    {
                        task_->info->downloadMap.set(i, true);
                        finishSize += task_->conf.bytesPerBlock;
                    }
                    else
                    {
                        task_->info->downloadMap.set(i, false);
                    }
                }
                int i = data.length() - 1;
                if (data[i] == '1')
                {
                    // need calculate last block seperately.
                    task_->info->downloadMap.set(i, true);
                    finishSize += task_->info->totalSize - (i * task_->conf.bytesPerBlock);
                }
                else
                {
                    task_->info->downloadMap.set(i, false);
                }
                task_->info->downloadSize = finishSize;
            }
            else
            {
                LOG(0, "can't handle <%s>%s</%s>\n", getElement(), data.c_str(), getElement());
            }
        }

    HttpTask *task_;

public:
    HttpConfXmlParser(HttpTask *task)
        : task_(task)
        {}

    std::ostringstream out;
};

void HttpProtocolData::initTask(HttpTask *task)
{
    LOG(0, "enter initTask, task = %p\n", task);

    HttpSession *ses = task->sessions[0];
    TaskInfo *info = task->info;
    CURL *ehandle = ses->handle;

    // get file information.
    char logBuffer[64] = {0};
    double length;
    if ( (info->outputName == NULL) || (info->outputName[0] == '\0') )
        getFileName(ehandle, task);
    std::string output = info->outputPath;
    output.append(info->outputName);

    if (info->totalSize == 0)
    {
        filesystem::File::remove(output.c_str());
    }

    task->file.open(output.c_str(), filesystem::Create | filesystem::Write);
    if (!task->file.isOpen())
        throw DOWNLOADEXCEPTION(FAIL_OPEN_FILE, "HTTP", strerror(FAIL_OPEN_FILE));
    snprintf(logBuffer, 63, "File path: %s", output.c_str());
    p->taskLog(info, logBuffer);

    if (info->totalSize == 0)
    {
        CURLcode rete = curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
        CHECK_CURLE(rete);
        if (length >= 0)
        {
            info->totalSize = static_cast<size_t>(length);
            info->downloadMap = BitMap(size_t(length), task->conf.bytesPerBlock);
            info->downloadMap.setAll(false);
            info->validMap = BitMap(size_t(length), task->conf.bytesPerBlock);
            info->validMap.setAll(true);
            snprintf(logBuffer, 63, "File length: %lu", info->totalSize);
            p->taskLog(info, logBuffer);

            task->file.resize(info->totalSize);
        }
    }

    if ( (info->totalSize > 0) && (task->conf.sessionNumber > 1) )
    {
        LOG(0, "make download sessions\n");
        // seperate download part in 2 steps:
        // 1 give each non-download part a session;
        // 2 find a biggest part and divide, repeat till enough session.
        unsigned int begin     = ses->pos / info->downloadMap.bytesPerBit();
        unsigned int end       = info->downloadMap.find(true, begin);
        unsigned int nextBegin = info->downloadMap.find(false, end);

        ses->length = (end - begin) * info->downloadMap.bytesPerBit();

        int i = 1;
        begin = nextBegin;

        while ( (i < task->conf.sessionNumber) && (begin < info->downloadMap.size()) )
        {
            end = info->downloadMap.find(true, begin);
            nextBegin = info->downloadMap.find(false, end);

            // make [begin, end) a seesion.
            makeSession(task, begin * info->downloadMap.bytesPerBit(), (end - begin) * info->downloadMap.bytesPerBit());

            ++i;
            begin = nextBegin;
        }

        while (i < task->conf.sessionNumber)
        {
            if (!splitMaxSession(task))
            {
                break;
            }

            ++i;
        }
    }

    task->state = HT_DOWNLOAD;
}

void HttpProtocolData::removeTask(const Tasks::iterator &taskIt)
{
    LOG(0, "enter removeTask\n");
    HttpTask *task = taskIt->second;
    assert(task != NULL);

    tasks.erase(taskIt);

    for (Sessions::iterator it = task->sessions.begin();
         it != task->sessions.end();
         ++it)
    {
        HttpSession *ses = *it;

        char logBuffer[64] = {0};
        snprintf(logBuffer, 63, "remove session at %lu", ses->pos);
        p->taskLog(task->info, logBuffer);

        if (ses->handle != NULL)
        {
            CURLMcode retm = curl_multi_remove_handle(handle, ses->handle);
            if (retm != CURLM_OK)
                throw DOWNLOADEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));

            curl_easy_cleanup(ses->handle);

            --task->info->validSource;
            --task->info->totalSource;
            --running;
        }

        ses->handle = NULL;
        ses->pos = 0;
        ses->length = 0;
    }
    task->sessions.clear();

    task->file.close();

    delete task;
}

void HttpProtocolData::saveTask(const Tasks::iterator &taskIt, std::ostream &out)
{
    HttpTask *task = taskIt->second;

    out << boost::format(
        "<SessionNumber>%d</SessionNumber>\n"
        "<MinSessionBlocks>%d</MinSessionBlocks>\n"
        "<BytesPerBlock>%d</BytesPerBlock>\n"
        "<TotalSize>%d</TotalSize>\n"
        "<BitMap>")
        % task->conf.sessionNumber
        % task->conf.minSessionBlocks
        % task->conf.bytesPerBlock
        % task->info->totalSize;

    for (size_t i=0, n=task->info->downloadMap.size(); i<n; ++i)
    {
        out << task->info->downloadMap.get(i);
    }

    out << "</BitMap>";
}

void HttpProtocolData::loadTask(HttpTask *task, std::istream &in)
{
    LOG(0, "enter loadTask, task = %p\n", task);

    HttpConfXmlParser parser(task);

    char buffer[1024];
    int size = 0;
    while ( (size = in.readsome(buffer, 1024)) > 0 )
    {
        parser.feed(buffer, size);

        if (parser.getError(NULL) != NULL)
        {
            break;
        }
    }
    parser.finish();

    TaskInfo *info = task->info;
    printf("download finish, total size = %lu\n", info->totalSize);
    printf("download finish, download size = %lu\n", info->downloadSize);
    printf("download map:\n");
    for (size_t i=0; i<info->downloadMap.size(); ++i)
    {
        printf("%d", info->downloadMap.get(i));
    }
    printf("\n");
}

bool findNonDownload(HttpTask *task, size_t *begin, size_t *len)
{
    std::vector<size_t> downloadBegins;
    for (Sessions::iterator it = task->sessions.begin();
         it != task->sessions.end();
         ++it)
    {
        downloadBegins.push_back((*it)->pos / task->info->downloadMap.bytesPerBit());
        LOG(0, "ses pos = %lu\n", downloadBegins.back());
    }

    size_t b = task->info->downloadMap.find(false, 0);
    while (b < task->info->downloadMap.size())
    {
        size_t e = task->info->downloadMap.find(true, b);
        if (find(downloadBegins.begin(), downloadBegins.end(), b) == downloadBegins.end())
        {
            // finded
            *begin = b * task->info->downloadMap.bytesPerBit();
            *len = (e - b) * task->info->downloadMap.bytesPerBit();

            return true;
        }
        b = task->info->downloadMap.find(false, e);
    }

    return false;
}

bool HttpProtocolData::splitMaxSession(HttpTask *task)
{
    LOG(0, "enter splitMaxSession, task = %p\n", task);

    if (task->sessions.size() == 0)
    {
        LOG(0, "no session find in task\n");
        return false;
    }

    HttpSession *maxLengthSes = task->sessions.front();

    for (Sessions::iterator it = task->sessions.begin() + 1;
         it != task->sessions.end();
         ++it)
    {
        if ((*it)->length > maxLengthSes->length)
        {
            maxLengthSes = *it;
        }
    }

    if ( maxLengthSes->length < (task->conf.minSessionBlocks * task->conf.bytesPerBlock * 2) )
    {
        LOG(0, "don't have enough length to split\n");
        return false;
    }

    size_t begin0 = maxLengthSes->pos / task->conf.bytesPerBlock;
    size_t length0 = maxLengthSes->length / task->conf.bytesPerBlock;
    size_t length = length0 / 2;
    size_t begin = (begin0 + length) * task->conf.bytesPerBlock;
    length0 = begin - maxLengthSes->pos;
    length = maxLengthSes->length - length0;
    maxLengthSes->length = length0;

    char logBuffer[64];
    snprintf(logBuffer, 63, "split session at %lu, length %d",
             maxLengthSes->pos, maxLengthSes->length);
    p->taskLog(task->info, logBuffer);

    makeSession(task, begin, length);

    return true;
}

void HttpProtocolData::checkSession(HttpSession *ses)
{
    LOG(0, "enter checkSession, ses = %p in task = %p\n", ses, ses->t);

    HttpTask *task = ses->t;

    LOG(0, "check task %d session from %lu, len %d, sessions size = %lu\n",
        task->info->id, ses->pos, ses->length, task->sessions.size());

    if (ses->length > 0)
        return;

    removeSession(ses);

    if (task->sessions.size() == 0)
    {
        LOG(0, "task %d finish\n", task->info->id);

        const TaskId id = task->info->id;
        p->downloadFinish(task->info);

        if (p->hasTask(id))
            p->removeTask(id);

        return;
    }

    size_t begin;
    size_t len;
    if (findNonDownload(task, &begin, &len))
    {
        makeSession(task, begin, len);
    }
    else
    {
        splitMaxSession(task);
    }
}

void HttpProtocolData::checkTasks()
{
    LOG(0, "check tasks\n");

    // check task status.
    CURLMsg *msg = NULL;
    int msgsInQueue;
    while ( (msg = curl_multi_info_read(handle, &msgsInQueue)) != NULL)
    {
        HttpSession *ses = NULL;
        CURLcode rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &ses);
        CHECK_CURLE(rete);

        long respCode = 0;
        rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &respCode);
        CHECK_CURLE(rete);
        int topRespCode = respCode / 100;
        LOG(0, "top response code = %d\n", topRespCode);

        switch (msg->msg)
        {
        case CURLMSG_DONE:
            switch (topRespCode)
            {
            case 2: // succeed download
                if (ses->t->state == HT_PREPARE)
                    initTask(ses->t);
                else
                {
                    ses->length = 0; // make sure session will be removed.
                    checkSession(ses);
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void HttpProtocolData::makeSession(HttpTask *task, size_t begin, size_t len)
{
    LOG(0, "make task %d session from %lu, len %lu\n", task->info->id, begin, len);
    char logBuffer[64] = {0};
    snprintf(logBuffer, 63, "make new session from %lu, len %lu", begin, len);
    p->taskLog(task->info, logBuffer);

    std::auto_ptr<HttpSession> ses( new HttpSession(task) );
    ses->pos = begin;
    size_t end = begin + len;
    if (task->info->totalSize < end)
        end = task->info->totalSize;
    ses->length = end - begin;

    ses->handle = curl_easy_init();
    if (ses->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(ses->handle, CURLOPT_URL, task->info->uri);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEDATA, ses.get());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    char range[128] = {0};
    sprintf(range, "%lu-%lu", begin, end - 1);
    rete = curl_easy_setopt(ses->handle, CURLOPT_RANGE, range);
    CHECK_CURLE(rete);

    CURLMcode retm = curl_multi_add_handle(this->handle, ses->handle);
    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }
    ++task->info->validSource;
    ++task->info->totalSource;
    ++running;

    task->sessions.push_back(ses.get());

    ses.release();
}

void HttpProtocolData::removeSession(HttpSession *ses)
{
    HttpTask *task = ses->t;
    LOG(0, "remove task %d session from %lu, len %d\n", task->info->id, ses->pos, ses->length);

    Sessions::iterator it = std::find(task->sessions.begin(),
                                      task->sessions.end(),
                                      ses);
#ifdef _DEBUG
    if (it == task->sessions.end())
    {
        LOG(0, "can't find ses\n");
        return;
    }
#endif
    char logBuffer[64] = {0};
    snprintf(logBuffer, 63, "remove session at %lu", ses->pos);
    p->taskLog(task->info, logBuffer);

    task->sessions.erase(it);

    if (ses->handle != NULL)
    {
        CURLMcode retm = curl_multi_remove_handle(handle, ses->handle);
        if (retm != CURLM_OK)
            throw DOWNLOADEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));

        curl_easy_cleanup(ses->handle);

        --task->info->validSource;
        --task->info->totalSource;
        --running;
    }

    ses->handle = NULL;
    ses->pos = 0;
    ses->length = 0;

    delete ses;
}

HttpProtocol::HttpProtocol()
    : d(new HttpProtocolData)
{
    LOG(0, "enter HttpProtocol ctor\n");

    d->p = this;

    CURLcode rete = curl_global_init(CURL_GLOBAL_ALL);
    if (rete != CURLE_OK)
        throw DOWNLOADEXCEPTION(rete, "CURLE", curl_easy_strerror(rete));

    d->handle = curl_multi_init();
    if (d->handle == 0)
        throw DOWNLOADEXCEPTION(CURLM_BAD_ALLOC, "CURLM", strerror(CURLM_BAD_ALLOC));
}

HttpProtocol::~HttpProtocol()
{
    LOG(0, "enter HttpProtocol dtor\n");
    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        d->removeTask(it);
    }

    CURLMcode retm = curl_multi_cleanup(d->handle);
    if (retm != CURLM_OK)
        throw DOWNLOADEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));

    curl_global_cleanup();
}

const char* HttpProtocol::name()
{
    return "HTTP";
}

bool nocaseEqual(char lhs, char rhs)
{
    if (lhs == rhs)
        return true;

    if ( ('A' <= lhs) && (lhs <= 'Z') )
        lhs = lhs - 'A' + 'a';
    // rhs will always lower case in this file when use.
    // if ( ('a' <= rhs) && (rhs <= 'z') )
    //    rhs -= 'a' - 'A';

    return lhs == rhs;
}

bool HttpProtocol::canProcess(const char *uri)
{
    // URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    // Common schemes include "file", "http", "svn+ssh", etc.

    char *scheme = strchr(uri, ':');
    const char *http = "http";

    while (uri != scheme)
    {
        if (!nocaseEqual(*uri, *http))
            return false;
        ++uri;
        ++http;
    }
    return true;
}

void HttpProtocol::loadOptions(std::istream &in)
{
    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");
}

void HttpProtocol::saveOptions(std::ostream &out)
{
    HttpConfigure defaultConf;

    out << boost::format(
        "<SessionNumber>%d</SessionNumber>\n"
        "<MinSessionBlocks>%d</MinSessionBlocks>\n"
        "<BytesPerBlock>%d</BytesPerBlock>")
        % defaultConf.sessionNumber
        % defaultConf.minSessionBlocks
        % defaultConf.bytesPerBlock;
}

const char* HttpProtocol::control(const char *cmd)
{
    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");

    return NULL;
}

const char* HttpProtocol::getAllOptions()
{
    HttpConfigure defaultConf;
    std::string buf = str(
        boost::format(
            "<SessionNumber>%d</SessionNumber>\n"
            "<MinSessionBlocks>%d</MinSessionBlocks>\n"
            "<BytesPerBlock>%d</BytesPerBlock>")
        % defaultConf.sessionNumber
        % defaultConf.minSessionBlocks
        % defaultConf.bytesPerBlock
        );

    static char *ret = NULL;
    if (ret != NULL)
        delete [] ret;

    ret = new char[buf.length() + 1];
    strcpy(ret, buf.c_str());

    return ret;
}

void HttpProtocol::addTask(TaskInfo *info)
{
    LOG(0, "enter addTask, info = %p\n", info);

    if (info == NULL)
        throw DOWNLOADEXCEPTION(NULL_INFO, "HTTP", strerror(NULL_INFO));

    Tasks::iterator it = d->tasks.find(info->id);
    if (it != d->tasks.end())
    {
        LOG(0, "task %d has exist\n", info->id);
        taskError(info, TASK_EXIST);
        return;
    }

    std::auto_ptr<HttpTask> task( new HttpTask(d.get()) );
    task->state = HT_PREPARE;
    task->info = info;
    task->info->protocol = this;
    task->info->downloadSize = 0;
    task->info->totalSource = task->info->validSource = 0;

    taskLog(task->info, "Add task, initialize");

    std::auto_ptr<HttpSession> ses( new HttpSession(task.get()) );

    ses->handle = curl_easy_init();
    if (ses->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(ses->handle, CURLOPT_URL, task->info->uri);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_FILETIME, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEDATA, ses.get());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    CURLMcode retm = curl_multi_add_handle(d->handle, ses->handle);
    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }
    ++d->running;

    d->tasks.insert(Tasks::value_type(info->id, task.get()));
    task->sessions.push_back(ses.get());

    ses.release();
    task.release();
}

void HttpProtocol::removeTask(const TaskId id)
{
    LOG(0, "enter removeTask, id = %u\n", id);

    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        LOG(0, "id %d doesn't exist\n", id);
        taskError(NULL, TASK_NOT_EXIST);
        return;
    }

    taskLog(it->second->info, "remove task");

    d->removeTask(it);
}

bool HttpProtocol::hasTask(const TaskId id)
{
    LOG(0, "enter hasTask, id = %u\n", id);

    Tasks::iterator it = d->tasks.find(id);
    return it != d->tasks.end();
}

void HttpProtocol::loadTask(TaskInfo *info, std::istream &in)
{
    LOG(0, "enter loadTask, info = %p\n", info);

    Tasks::iterator it = d->tasks.find(info->id);
    if (it != d->tasks.end())
    {
        LOG(0, "id %d exist\n", info->id);
        taskError(info, TASK_EXIST);
        return;
    }

    std::auto_ptr<HttpTask> task( new HttpTask(d.get()) );
    task->info = info;
    task->info->protocol = this;
    task->state = HT_PREPARE;

    d->loadTask(task.get(), in);

    unsigned int begin = info->downloadMap.find(false, 0) * task->info->downloadMap.bytesPerBit();
    unsigned int end   = info->downloadMap.find(true, begin) * task->info->downloadMap.bytesPerBit();

    std::auto_ptr<HttpSession> ses( new HttpSession(task.get()) );
    ses->pos = begin;
    ses->length = end - begin;

    ses->handle = curl_easy_init();
    if (ses->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(ses->handle, CURLOPT_URL, task->info->uri);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_FILETIME, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEDATA, ses.get());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    if (task->info->totalSize > 0)
    {
        char range[64];
        sprintf(range, "%u-%u", begin, end);
        LOG(0, "resume task at %s\n", range);

        rete = curl_easy_setopt(ses->handle, CURLOPT_RANGE, range);
        CHECK_CURLE(rete);
    }

    CURLMcode retm = curl_multi_add_handle(d->handle, ses->handle);
    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }
    ++d->running;

    d->tasks.insert(Tasks::value_type(info->id, task.get()));
    task->sessions.push_back(ses.get());

    taskLog(info, "load task");

    ses.release();
    task.release();
}

void HttpProtocol::saveTask(const TaskId id, std::ostream &out)
{
    LOG(0, "enter saveTask, id = %u\n", id);

    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        LOG(0, "id %d doesn't exist\n", id);
        taskError(NULL, TASK_NOT_EXIST);
        return;
    }

    taskLog(it->second->info, "save task");

    d->saveTask(it, out);
}

const char* HttpProtocol::controlTask(const TaskId id, const char *cmd)
{
    LOG(0, "enter controlTask, id = %u\n", id);

    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");

    return NULL;
}

void HttpProtocol::getFdSet(fd_set *read_fd_set,
                            fd_set *write_fd_set,
                            fd_set *exc_fd_set,
                            int *max_fd)
{
    LOG(0, "enter getFdSet, read_fd_set = %p, write_fd_set = %p, exc_fd_set = %p, max_fd = %p\n",
        read_fd_set, write_fd_set, exc_fd_set, max_fd);

    // check task status.
    CURLMcode retm = curl_multi_fdset(d->handle,
                                      read_fd_set,
                                      write_fd_set,
                                      exc_fd_set,
                                      max_fd);

    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }
}

int HttpProtocol::perform()
{
    int running;
    CURLMcode retm = CURLM_OK;

    while ( (retm = curl_multi_perform(d->handle, &running)) ==
            CURLM_CALL_MULTI_PERFORM )
    {}

    if (retm != CURLM_OK)
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));

    if (running != d->running)
    {
        LOG(0, "running = %d / %d\n", d->running, running);
        d->checkTasks();
        d->running = running;
    }

    for (Sessions::iterator it = d->finishSessions.begin();
         it != d->finishSessions.end();
         ++it)
    {
        LOG(0, "running = %d / %d\n", d->running, running);
        d->checkSession(*it);
    }
    d->finishSessions.clear();

    return d->tasks.size();
}

const char* HttpProtocol::strerror(int error)
{
    LOG(0, "enter strerror, error = %d\n", error);

    static const char *errorString[] =
        {
            "alloc curl easy handle fail.",      // CURL_BAD_ALLOC
            "alloc curl multi handle fail.",     // CURLM_BAD_ALLOC
            "pass NULL task info.",              // NULL_INFO
            "bad length of file in write back.", // BAD_FILE_LENGTH
            "open file failed.",                 // FAIL_OPEN_FILE
            "task exist.",                       // TASK_EXIST
            "task not exist.",                   // TASK_NOT_EXIST
        };

    if ( (CURL_BAD_ALLOC <= error) && (error <= FAIL_OPEN_FILE) )
        return errorString[error];
    else
        return "unknown error.";
}
