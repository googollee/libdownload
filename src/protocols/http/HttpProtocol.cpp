/**
 * HttpProtocol Structure Map:
 *
 *                    +------------+           +--------+
 *                    |HttpTaskInfo|<>---------|TaskInfo|
 *                    +------------+           +--------+
 *             +------|int fd      |
 *             |      +------------+
 *             |           <>
 *             |            | 1..n
 *             |            |
 *             |            |
 *             |    +----------------+
 *             |    |    Session     |
 *             |    +----------------+
 *             |    |size_t writePos |---+
 *             |    |size_t length   |   |
 *             |    +----------------+   |
 *             |           <>            |
 *             |            | 1          |
 *             |            |            |
 *             |            |            |
 *             |    +----------------+   |
 *             +--<>|curl_easy_handle|<>-+ WRITEFUNCTION_DATA
 *   PRIVATE_DATA   +----------------+
 */

#include "HttpProtocol.h"
#include "HttpProtocolImpl.h"

#include <utility/DownloadException.h>

#include <curl/curl.h>

#include <cstring>
#include <string>
#include <algorithm>

#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
            throw DOWNLOADEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

using std::vector;
using std::map;
using std::string;

using filesystem::File;

enum HttpError
{
    CURL_BAD_ALLOC,
    CURL_SLIST_BAD_ALLOC,
    CURLM_BAD_ALLOC,
    NULL_INFO,
    BAD_FILE_LENGTH,
    FAIL_OPEN_FILE,
};

static const char* strerror(HttpError error);

const char* strerror(HttpError error)
{
    static const char *errorString[] =
        {
            "alloc curl easy handle fail.",  // CURL_BAD_ALLOC
            "alloc curl slist fail.",        // CURL_SLIST_BAD_ALLOC
            "alloc curl multi handle fail.", // CURLM_BAD_ALLOC
            "pass NULL task info.",          // NULL_INFO
            "bad length of file in write back", //BAD_FILE_LENGTH
            "open file failed",              // FAIL_OPEN_FILE
        };

    return errorString[error];
}

static unsigned int downloadSize;

size_t writeCallback(void *buffer, size_t size, size_t nmemb, HttpSessionInfo *sinfo)
{
    size_t totalSize = size * nmemb;

    sinfo->parent->file->seek(sinfo->writePos, filesystem::FromBegin);
    size_t writed = sinfo->parent->file->write(buffer, totalSize);

    sinfo->parent->taskInfo->bitMap.setRangeByLength(sinfo->writePos, sinfo->writePos + writed, true);
    sinfo->length -= writed;
    sinfo->writePos += writed;

    downloadSize += totalSize;

    return writed;
}

inline void HttpProtocolData::eraseSession(HttpSessionInfo *sinfo)
{
    if (sinfo->handle != NULL)
    {
        CURLMcode retm = curl_multi_remove_handle(handle, sinfo->handle);
        if (retm != CURLM_OK)
            throw DOWNLOADEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));

        curl_easy_cleanup(sinfo->handle);
    }

    sinfo->handle = NULL;
    sinfo->writePos = 0;
    sinfo->length = 0;
    sinfo->parent = NULL;
    delete sinfo;
 }

inline void HttpProtocolData::delTask(const Tasks::iterator &taskIt)
{
    assert(taskIt->second->taskInfo != NULL);
    for (Sessions::iterator it = taskIt->second->sessions.begin();
         it != taskIt->second->sessions.end();
         ++it)
    {
        eraseSession(*it);
    }

    if (taskIt->second->file != NULL)
        delete taskIt->second->file;

    tasks.erase(taskIt);

    delete taskIt->second;
}

inline void HttpProtocolData::saveTask(const Tasks::iterator &taskIt,
                                       std::ostream_iterator<char> &out)
{
    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");
}

inline void HttpProtocolData::loadTask(const Tasks::iterator &taskIt,
                                       std::istream_iterator<char> &begin,
                                       std::istream_iterator<char> &end)
{
    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");
}

bool findNonDownload(HttpTaskInfo *info, size_t *begin, size_t *len)
{
    vector<size_t> downloadBegins;
    for (Sessions::iterator it = info->sessions.begin();
         it != info->sessions.end();
         ++it)
    {
        downloadBegins.push_back((*it)->writePos / info->taskInfo->bitMap.bytesPerBit());
    }

    size_t b = info->taskInfo->bitMap.find(false, 0);
    while (b < info->taskInfo->bitMap.size())
    {
        size_t e = info->taskInfo->bitMap.find(true, b);
        if (find(downloadBegins.begin(), downloadBegins.end(), b) == downloadBegins.end())
        {
            // finded
            *begin = b * info->taskInfo->bitMap.bytesPerBit();
            *len = (e - b) * info->taskInfo->bitMap.bytesPerBit();

            return true;
        }
        b = info->taskInfo->bitMap.find(false, e);
    }

    return false;
}

void HttpProtocolData::checkTasks()
{
    // check task status.
    CURLMsg *msg = NULL;
    int msgsInQueue;
    while ( (msg = curl_multi_info_read(handle, &msgsInQueue)) != NULL)
    {
        HttpSessionInfo *sinfo = NULL;
        CURLcode rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &sinfo);
        CHECK_CURLE(rete);

        HttpTaskInfo *hinfo = sinfo->parent;

        long respCode = 0;
        rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &respCode);
        CHECK_CURLE(rete);
        int topRespCode = respCode / 100;
        printf("top response code = %d\n", topRespCode);

        switch (msg->msg)
        {
        case CURLMSG_DONE:
            switch (topRespCode)
            {
            case 2: // succeed download
                sinfo->parent->sessions.erase(
                    std::find(sinfo->parent->sessions.begin(),
                              sinfo->parent->sessions.end(),
                              sinfo)
                    );
                eraseSession(sinfo);
                {
                    size_t begin;
                    size_t len;
                    if (findNonDownload(hinfo, &begin, &len))
                    {
                        makeSession(hinfo, begin, len);
                    }
                }
                if (hinfo->sessions.size() == 0)
                {
                    printf("task %s download finish\n", hinfo->taskInfo->url);
                }
                break;
            }
            break;
        default:
            break;
        }
    }
}

TaskId HttpProtocolData::getNewID()
{
    typedef vector<TaskId> IDs;
    IDs ids(tasks.size(), -1);

    // insert sort
    for (Tasks::iterator it = tasks.begin();
         it != tasks.end();
         ++it)
    {
        IDs::iterator insertPos = ids.begin();
        while (insertPos != ids.end())
        {
            if (it->first > *insertPos)
                break;
            ++it;
        }
        ids.insert(insertPos, it->first);
    }

    TaskId ret = 0;
    for (IDs::iterator it = ids.begin();
         it != ids.end();
         ++it)
    {
        if (ret != *it)
            break;
        ++ret;
    }

    return ret;
}

void HttpProtocolData::makeSession(HttpTaskInfo *info, size_t begin, size_t len)
{
    HttpSessionInfo *sinfo = new HttpSessionInfo;
    sinfo->parent = info;
    sinfo->writePos = begin;
    size_t end = begin + len;
    if (info->taskInfo->totalSize < end)
        end = info->taskInfo->totalSize;
    sinfo->length = end - begin;

    sinfo->handle = curl_easy_init();
    if (sinfo->handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(sinfo->handle, CURLOPT_URL, info->taskInfo->url);

    rete = curl_easy_setopt(sinfo->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(sinfo->handle, CURLOPT_WRITEDATA, sinfo);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(sinfo->handle, CURLOPT_PRIVATE, sinfo);
    CHECK_CURLE(rete);

    char range[128] = {0};
    sprintf(range, "%lu-%lu", begin, end - 1);
    rete = curl_easy_setopt(sinfo->handle, CURLOPT_RANGE, range);
    CHECK_CURLE(rete);

    CURLMcode retm = curl_multi_add_handle(this->handle, sinfo->handle);
    if (retm != CURLM_OK)
    {
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }

    info->sessions.push_back(sinfo);
}

HttpProtocol::HttpProtocol()
    : d(new HttpProtocolData)
{
    CURLcode rete = curl_global_init(CURL_GLOBAL_ALL);
    if (rete != CURLE_OK)
        throw DOWNLOADEXCEPTION(rete, "CURLE", curl_easy_strerror(rete));

    d->handle = curl_multi_init();
    if (d->handle == 0)
        throw DOWNLOADEXCEPTION(CURLM_BAD_ALLOC, "CURLM", strerror(CURLM_BAD_ALLOC));
}

HttpProtocol::~HttpProtocol()
{
    for (Tasks::iterator it = d->tasks.begin();
         it != d->tasks.end();
         ++it)
    {
        d->delTask(it);
    }

    CURLMcode retm = curl_multi_cleanup(d->handle);
    if (retm != CURLM_OK)
        throw DOWNLOADEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));

    curl_global_cleanup();
}

const char* HttpProtocol::name()
{
    return "HTTP download";
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

void HttpProtocol::setOptions(const char *opts)
{
    throw DOWNLOADEXCEPTION(1, "HTTP", "not implement");
}

const char* HttpProtocol::getAllOptions()
{
    return "<sessionNumber default=\"5\" />";
}

void getFileName(CURL *handle, HttpTaskInfo *httpInfo)
{
    const char *p = httpInfo->taskInfo->url + strlen(httpInfo->taskInfo->url);
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
    } while ( !findNonNameChar && (p >= httpInfo->taskInfo->url) );
    ++p;

    if (httpInfo->taskInfo->outputName != NULL)
        delete [] httpInfo->taskInfo->outputName;

    httpInfo->taskInfo->outputName = strdup(p);
}

void initTaskInfo(HttpTaskInfo *info)
{
    CURL *handle = curl_easy_init();
    if (handle == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(handle, CURLOPT_URL, info->taskInfo->url);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle, CURLOPT_FILETIME, 1);
    CHECK_CURLE(rete);

    // here need changed as asio.
    rete = curl_easy_perform(handle);
    CHECK_CURLE(rete);

    double length;
    rete = curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
    CHECK_CURLE(rete);
    info->taskInfo->totalSize = static_cast<size_t>(length);
    info->taskInfo->finishSize = 0;
    info->taskInfo->bitMap = BitMap(size_t(length), DefaultBytePerBlock);
    info->taskInfo->bitMap.setAll(false);

    long remoteTime;
    rete = curl_easy_getinfo(handle, CURLINFO_FILETIME, &remoteTime);
    CHECK_CURLE(rete);
    info->remoteTime = static_cast<time_t>(remoteTime);

    char *mimeType;
    rete = curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &mimeType);
    CHECK_CURLE(rete);
    info->mimeType = mimeType;

    if ( (info->taskInfo->outputName == NULL) || (info->taskInfo->outputName[0] == '\0') )
        getFileName(handle, info);
    string output = info->taskInfo->outputPath;
    output.append(info->taskInfo->outputName);
    info->file = new filesystem::File(output.c_str(), filesystem::Create | filesystem::Write);
    if (!info->file->isOpen())
        throw DOWNLOADEXCEPTION(FAIL_OPEN_FILE, "HTTP", strerror(FAIL_OPEN_FILE));

    info->file->resize(info->taskInfo->totalSize);

    curl_easy_cleanup(handle);
}

void HttpProtocol::addTask(const TaskId id, TaskInfo *info)
{
    if (info == NULL)
        throw DOWNLOADEXCEPTION(NULL_INFO, "HTTP", strerror(NULL_INFO));

    Tasks::iterator it = d->tasks.find(id);
    if (it != d->tasks.end())
    {
        printf("id %d has exist\n", id);
        return;
    }

    HttpTaskInfo *httpInfo = new HttpTaskInfo();
    httpInfo->taskInfo = info;

    initTaskInfo(httpInfo);

    unsigned int sessionNumber = d->defaultSessionNumber;
    unsigned int begin = 0;
    unsigned int end = 0;
    unsigned int nextBegin = 0;
    unsigned int i = 0;
    while ( (i<sessionNumber) && (begin < info->bitMap.size()) )
    {
        end = info->bitMap.find(true, begin);
        nextBegin = info->bitMap.find(false, end);
        if ( (end == info->bitMap.size()) || (nextBegin == info->bitMap.size()) )
        {
            // devide [begin, end) to sessionNumber-i directly.
            int n = sessionNumber - i;
            while (begin < end)
            {
                int len = (end - begin) / n;
                d->makeSession(httpInfo, begin * info->bitMap.bytesPerBit(), len * info->bitMap.bytesPerBit());
                begin += len;
                --n;
            }
            i = sessionNumber;
        }
        else
        {
            // make [begin, end) a seesion.
            d->makeSession(httpInfo, begin * info->bitMap.bytesPerBit(), (end - begin) * info->bitMap.bytesPerBit());
            ++i;
        }
        begin = nextBegin;
    }

    d->tasks.insert(Tasks::value_type(id, httpInfo));
}

void HttpProtocol::delTask(const TaskId id)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->delTask(it);
}

bool HttpProtocol::hasTask(const TaskId id)
{
    Tasks::iterator it = d->tasks.find(id);
    return it != d->tasks.end();
}

void HttpProtocol::saveTask(const TaskId id,
                            std::ostream_iterator<char> &out)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->saveTask(it, out);
}

void HttpProtocol::loadTask(const TaskId id,
                            std::istream_iterator<char> &begin,
                            std::istream_iterator<char> &end)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->loadTask(it, begin, end);
}

void HttpProtocol::getFdSet(fd_set *read_fd_set,
                            fd_set *write_fd_set,
                            fd_set *exc_fd_set,
                            int *max_fd)
{
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

size_t HttpProtocol::perform(size_t *downloaded, size_t *uploaded)
{
    int running;
    CURLMcode retm = CURLM_OK;

    downloadSize = 0;
    while ( (retm = curl_multi_perform(d->handle, &running)) ==
            CURLM_CALL_MULTI_PERFORM )
    {}

    if (retm != CURLM_OK)
        throw DOWNLOADEXCEPTION(retm, "CURL", curl_multi_strerror(retm));

    if ( (d->running > 0) && (running < d->running) )
    {
        d->checkTasks();
    }
    d->running = running;

    *downloaded = downloadSize;
    *uploaded = 0;

    return running;
}

const char* HttpProtocol::strerror(int error)
{
    return "";
}
