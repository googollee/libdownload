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
#include <utility/SingleCurlHelper.h>

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
    XML_PARSE_ERROR,
    LAST_ERROR = XML_PARSE_ERROR,
};

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
            "xml parse error.",                  // XML_PARSE_ERROR
        };

    if ( (CURL_BAD_ALLOC <= error) && (error <= LAST_ERROR) )
        return errorString[error];
    else
        return "unknown error.";
}

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
        task->file.seek(ses->pos, SF_FromBegin);
    }

    const size_t writed = task->file.write(buffer, shouldWrite);

    task->info->downloadSize += writed;
    task->d->performSize += writed;

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

std::string getFileName(CURL *handle, const std::string &uri)
{
    size_t p = uri.find_last_of("*|\\:\"<>?/") + 1;
    return uri.substr(p);
}

std::string getMimeType(CURL *handle, const std::string &uri)
{
    char *contentType = NULL;
    std::string ret;
    curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &contentType);

    if (contentType != NULL)
    {
        char *end = strchr(contentType, ';');
        if (end != NULL)
        {
            ret = std::string(contentType, end - contentType);
        }
        else
        {
            ret = contentType;
        }
    }

    return ret;
}

void HttpProtocolData::initTask(HttpTask *task)
{
    LOG(0, "enter initTask, task = %p\n", task);

    HttpSession *ses = task->sessions[0];
    TaskInfo *info = task->info;
    CURL *ehandle = ses->handle;

    char logBuffer[64] = {0};

    info->mimeType = getMimeType(ehandle, info->uri);
    snprintf(logBuffer, 63, "mime type: %s", info->mimeType.c_str());
    info->taskLog(info, logBuffer);

    // get file information.
    double length;
    if (info->outputName.length() == 0)
        info->outputName = getFileName(ehandle, info->uri);

    std::string output = info->outputPath;
    output.append(info->outputName);

    if (info->totalSize == 0)
    {
        File::remove(output.c_str());
    }

    task->file.open(output.c_str(), OF_Create | OF_Write);
    if (!task->file.isOpen())
        throw DOWNLOADEXCEPTION(FAIL_OPEN_FILE, "HTTP", strerror(FAIL_OPEN_FILE));
    snprintf(logBuffer, 63, "File path: %s", output.c_str());
    info->taskLog(info, logBuffer);

    if (info->totalSize == 0)
    {
        CURLcode rete = curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
        CHECK_CURLE(rete);
        if (length > 0)
        {
            info->totalSize = static_cast<size_t>(length);
            info->downloadMap = BitMap(size_t(length), task->conf.bytesPerBlock);
            info->downloadMap.setAll(false);
            info->validMap = BitMap(size_t(length), task->conf.bytesPerBlock);
            info->validMap.setAll(true);
            snprintf(logBuffer, 63, "File length: %lu", info->totalSize);
            info->taskLog(info, logBuffer);

            task->file.resize(info->totalSize);
        }
        else if (length < 0)
            info->totalSize = 0;
        else // length == 0, zero length file
            return;
    }

    info->totalSource = info->validSource = 1;

    if (info->totalSize > 0) // may equal 0 mean unknow length.
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
        task->info->taskLog(task->info, logBuffer);

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
    task->sessions.clear();
    // task->file will be automatically closed.

    delete task;
}

void HttpProtocolData::saveTask(const Tasks::iterator &taskIt, std::string &data)
{
    HttpTask *task = taskIt->second;

    data.clear();

    // check if it can resume.
    if (task->info->totalSize == 0)
        return;

    std::stringstream out;
    out << boost::format(
        "<MinSessionBlocks>%d</MinSessionBlocks>"
        "<BytesPerBlock>%d</BytesPerBlock>"
        "<TotalSize>%d</TotalSize>"
        "<BitMap>")
        % task->conf.minSessionBlocks
        % task->conf.bytesPerBlock
        % task->info->totalSize;

    for (size_t i=0, n=task->info->downloadMap.size(); i<n; ++i)
    {
        out << task->info->downloadMap.get(i);
    }

    out << "</BitMap>";

    data = out.str();
}

void HttpProtocolData::loadTask(HttpTask *task, std::string &data)
{
    LOG(0, "enter loadTask, task = %p\n", task);

    TaskInfo *info = task->info;
    HttpConfXmlParser parser;

    parser.feed(data.c_str());
    if (parser.getError(NULL) != NULL)
    {
        LOG(0, "load task %p error: %s\n", task, parser.getError(NULL));
        throw DOWNLOADEXCEPTION(XML_PARSE_ERROR, "HTTP", parser.getError(NULL));
    }
    parser.finish();

    if (parser.conf.sessionNumber > 0)
        task->conf.sessionNumber = parser.conf.sessionNumber;
    if (parser.conf.minSessionBlocks > 0)
        task->conf.minSessionBlocks = parser.conf.minSessionBlocks;
    if (parser.conf.bytesPerBlock > 0)
        task->conf.bytesPerBlock = parser.conf.bytesPerBlock;
    if (parser.conf.referer.length() > 0)
        task->conf.referer = parser.conf.referer;
    if (parser.conf.userAgent.length() > 0)
        task->conf.userAgent = parser.conf.userAgent;
    if (parser.totalSize > 0)
        info->totalSize = parser.totalSize;
    if (parser.downloadMap.size() > 0)
    {
        info->downloadSize = parser.downloadSize;
        info->downloadMap = parser.downloadMap;
    }
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
    task->info->taskLog(task->info, logBuffer);

    makeSession(task, begin, length);

    return true;
}

void HttpProtocolData::checkSession(HttpSession *ses)
{
    LOG(0, "enter checkSession, ses = %p in task = %p\n", ses, ses->t);

    HttpTask *task = ses->t;

    LOG(0, "check task %p session from %lu, len %d, sessions size = %lu\n",
        task->info, ses->pos, ses->length, task->sessions.size());

    if (ses->length > 0)
    {
        LOG(0, "session length is not 0.\n");
    }

    removeSession(ses);

    size_t begin;
    size_t len;
    if (findNonDownload(task, &begin, &len))
    {
        makeSession(task, begin, len);
    }
    else if (task->sessions.size() == 0)
    {
        TaskInfo *info = task->info;
        LOG(0, "task %p finish\n", info);

        task->info->downloadFinish(info);

        p->removeTask(info);
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
                {
                    initTask(ses->t);
                }

                ses->length = 0; // make sure session will be removed.
                checkSession(ses);
                break;
            default:
                break;
            }
            break;
            // TODO: handle other return code.
        default:
            break;
        }
    }
}

void HttpProtocolData::makeSession(HttpTask *task, size_t begin, size_t len)
{
    LOG(0, "make task %p session from %lu, len %lu\n", task->info, begin, len);
    char logBuffer[64] = {0};
    snprintf(logBuffer, 63, "make new session from %lu, len %lu", begin, len);
    task->info->taskLog(task->info, logBuffer);

    std::auto_ptr<HttpSession> ses( new HttpSession(task) );
    ses->pos = begin;
    size_t end = begin + len;
    if (task->info->totalSize < end)
        end = task->info->totalSize;
    ses->length = end - begin;

    ses->handle = curl_easy_init();
    if (ses->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(ses->handle, CURLOPT_URL, task->info->uri.c_str());

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

    // TODO: maybe need short sessions by pos.
    task->sessions.push_back(ses.get());

    ses.release();
}

void HttpProtocolData::removeSession(HttpSession *ses)
{
    HttpTask *task = ses->t;
    LOG(0, "remove task %p session from %lu, len %d\n", task->info, ses->pos, ses->length);

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
    task->info->taskLog(task->info, logBuffer);

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

    SingleCurlHelper::init();

    d->p = this;

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

const char* HttpProtocol::getOptionsDetail()
{
    return
        "<SessionNumber>"
            "<title>Session number</title>"
            "<desc>How many sessions in one task</desc>"
            "<type>unsignedInt</type>"
            "<pattern/>"
        "</SessionNumber>"
        "<MinSessionBlocks>"
            "<title>Min session blocks</title>"
            "<desc>The minimum blocks of one session</desc>"
            "<type>unsignedInt</type>"
            "<pattern/>"
        "</MinSessionBlocks>"
        "<BytesPerBlock>"
            "<title>Bytes per block</title>"
            "<desc>How many bytes in a block</desc>"
            "<type>unsignedInt</type>"
            "<pattern/>"
        "</BytesPerBlock>"
        "<Referer>"
            "<title>Referer uri</title>"
            "<desc>The referer in http request sent to remote server</desc>"
            "<type>string</type>"
            "<pattern/>"
        "</Referer>"
        "<UserAgent>"
            "<title>User agent</title>"
            "<desc>The user agent in http request sent to remote server</desc>"
            "<type>string</type>"
            "<pattern/>"
        "</UserAgent>";
}

const char* HttpProtocol::getOptions()
{
    static char buffer[64];

    snprintf(buffer, 63,
             "<SessionNumber>%d</SessionNumber>",
             d->defaultConf.sessionNumber);

    return buffer;
}

void HttpProtocol::setOptions(const char *options)
{
    HttpConfXmlParser parser;

    parser.feed(options);
    parser.finish();

    if (parser.getError(NULL) != NULL)
    {
        char buffer[64] = {0};
        snprintf(buffer, 63, "setOptions fail: %s", parser.getError(NULL));
        protocolLog(this, buffer);
        return;
    }

    if (parser.conf.sessionNumber > 0)
        d->defaultConf.sessionNumber = parser.conf.sessionNumber;
}

const char* HttpProtocol::getTaskOptions(const char *uri)
{
    static char *ret = NULL;
    static size_t s = 0;

    std::string buffer = str(
        boost::format("<SessionNumber>%d</SessionNumber>"
                      "<Referer>%s</Referer>"
                      "<UserAgent>%s</UserAgent>")
        % d->defaultConf.sessionNumber
        % d->defaultConf.referer
        % d->defaultConf.userAgent);

    if (ret == NULL)
    {
        s = buffer.length() + 1;
        ret = new char[s];
    }
    else if ( (buffer.length() + 1) > s )
    {
        delete [] ret;
        s = buffer.length() + 1;
        ret = new char[s];
    }
    strcpy(ret, buffer.c_str());

    return ret;
}

void HttpProtocol::addTask(TaskInfo *info)
{
    LOG(0, "enter addTask, info = %p\n", info);

    if (info == NULL)
        throw DOWNLOADEXCEPTION(NULL_INFO, "HTTP", strerror(NULL_INFO));

    Tasks::iterator it = d->tasks.find(info);
    if (it != d->tasks.end())
    {
        LOG(0, "task %p has exist\n", info);
        throw DOWNLOADEXCEPTION(TASK_EXIST, "HTTP", strerror(TASK_EXIST));
    }

    std::auto_ptr<HttpTask> task( new HttpTask(d.get()) );
    task->state = HT_PREPARE;
    task->info = info;
    info->protocol = this;
    info->downloadSize = 0;
    info->totalSource = info->validSource = 0;

    std::auto_ptr<HttpSession> ses( new HttpSession(task.get()) );

    ses->handle = curl_easy_init();
    if (ses->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(ses->handle, CURLOPT_URL, task->info->uri.c_str());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_FILETIME, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_WRITEDATA, ses.get());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    if (info->processData.length() != 0)
    {
        info->taskLog(task->info, "Resume task, initialize");

        d->loadTask(task.get(), info->processData);

        if (task->info->totalSize > 0)
        {
            unsigned int begin = info->downloadMap.find(false, 0) * task->info->downloadMap.bytesPerBit();
            unsigned int end   = info->downloadMap.find(true, begin) * task->info->downloadMap.bytesPerBit();
            ses->pos = begin;
            ses->length = end - begin;

            char range[64];
            sprintf(range, "%u-%u", begin, end);
            LOG(0, "resume task at %s\n", range);

            rete = curl_easy_setopt(ses->handle, CURLOPT_RANGE, range);
            CHECK_CURLE(rete);
        }
    }
    else
    {
        info->taskLog(task->info, "Add new task, initialize");
    }

    if (info->options.length() > 0)
    {
        d->loadTask(task.get(), info->options);
    }

    if (task->conf.referer.length() > 0)
    {
        rete = curl_easy_setopt(ses->handle, CURLOPT_REFERER, task->conf.referer.c_str());
        CHECK_CURLE(rete);
    }

    if (task->conf.userAgent.length() > 0)
    {
        rete = curl_easy_setopt(ses->handle, CURLOPT_USERAGENT, task->conf.userAgent.c_str());
        CHECK_CURLE(rete);
    }

    CURLMcode retm = curl_multi_add_handle(d->handle, ses->handle);
    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }
    ++d->running;

    d->tasks.insert(Tasks::value_type(info, task.get()));
    task->sessions.push_back(ses.get());

    ses.release();
    task.release();
}

void HttpProtocol::flushTask(TaskInfo *info)
{
    LOG(0, "enter flushTask, info = %p\n", info);

    Tasks::iterator it = d->tasks.find(info);
    if (it == d->tasks.end())
    {
        LOG(0, "task %p doesn't exist\n", info);
        info->taskError(this, info, TASK_NOT_EXIST);
        return;
    }

    info->taskLog(info, "save task");

    d->saveTask(it, info->processData);
}

void HttpProtocol::removeTask(TaskInfo *info)
{
    LOG(0, "enter removeTask, info = %p\n", info);

    Tasks::iterator it = d->tasks.find(info);
    if (it == d->tasks.end())
    {
        LOG(0, "task %p doesn't exist\n", info);
        info->taskError(this, info, TASK_NOT_EXIST);
        return;
    }

    info->taskLog(info, "remove task");

    d->removeTask(it);
}

bool HttpProtocol::hasTask(TaskInfo *info)
{
    LOG(0, "enter hasTask, info = %p\n", info);

    Tasks::iterator it = d->tasks.find(info);
    return it != d->tasks.end();
}

const char* HttpProtocol::getTaskState(TaskInfo *info)
{
    LOG(0, "enter getTaskState, info = %p\n", info);

    // TODO: return the state of all sessions.
    return "";
}

void HttpProtocol::getFdSet(fd_set *read_fd_set,
                            fd_set *write_fd_set,
                            fd_set *exc_fd_set,
                            int    *max_fd)
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

int HttpProtocol::performDownload(size_t *size)
{
    int running;
    CURLMcode retm = CURLM_OK;
    d->performSize = 0;

    while ( (retm = curl_multi_perform(d->handle, &running)) ==
            CURLM_CALL_MULTI_PERFORM )
    {}

    if (size != NULL)
        *size = d->performSize;

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

int HttpProtocol::performUpload(size_t *size)
{
    // Http protocol doens't support upload now.
    if (size != NULL)
        *size = 0;
    return 0;
}
