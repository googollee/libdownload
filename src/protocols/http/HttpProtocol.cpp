#include "HttpProtocol.h"
#include "HttpProtocolImpl.h"
#include "../ProtocolException.h"

#include <curl/curl.h>

#include <cstring>
#include <string>
#include <algorithm>

using std::vector;
using std::map;
using std::string;

static const char* HttpProtocol::strerror(HttpError error)
{
    static const char **errorString =
        {
            "alloc curl easy handle fail.",  // CURL_BAD_ALLOC
            "alloc curl slist fail.",        // CURL_SLIST_BAD_ALLOC
            "alloc curl multi handle fail.", // CURLM_BAD_ALLOC
            "pass NULL task info.",          // NULL_INFO
            "bad length of file in write back", //BAD_FILE_LENGTH
        };

    return errorString[error];
}

inline void HttpProtocolData::delTask(Tasks::iterator &taskIt)
{
    assert(it->info != NULL);
    for (Sessions::iterator it = taskIt->second->sessions.begin();
         it != taskIt->second->sessions.end();
         ++it)
    {
        CURLMcode retm = curl_multi_remove_handle(handle, it->second->handle);
        if (retm != CURLM_OK)
            throw PROTOCOLEXCEPTIOIN(retm, "CURLM", curl_multi_strerror(retm));

        curl_easy_cleanup(it->second->handle);
    }

    if (taskIt->second->file != NULL)
        fclose(taskIt->second->file);

    tasks.erase(taskIt);
}

inline void HttpProtocolData::saveTask(const Tasks::iterator &taskIt,
                                       std::ostream_iterator &out)
{
    throw PROTOCOLEXCEPTOIN(1, "HTTP", "not implement");
}

inline void HttpProtocolData::saveTask(const Tasks::iterator &taskIt,
                                       std::istream_iterator &begin,
                                       std::istream_iterator &end)
{
    throw PROTOCOLEXCEPTOIN(1, "HTTP", "not implement");
}

size_t writeCallback(void *buffer, size_t size, size_t nmemb, HttpSessionInfo *sinfo)
{
    size_t totalSize = size * nmemb;
    if (totalSize > sinfo->length)
        throw PROTOCOLEXCEPTION(HttpProtocol::BAD_FILE_LENGTH,
                                "HTTP",
                                HttpProtocol::strerror(BAD_FILE_LENGTH));

    sinfo->parent->file.seek(sinfo.writePos);
    sinfo->parent->file.write(buffer, totalSize);

    sinfo->parent->info->bitMap.setRangeByLength(sinfo->writePos, totalSize);
    sinfo->length -= totalSize;
    sinfo->writePos += totalSize;

    return totalSize;
}

HttpProtocol::HttpProtocol()
    : d(new HttpProtocolData)
{
    d->defaultSessionNumber = DefaultSessionNumber;

    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);

    if (rete != CURLE_OK)
        throw PROTOCOLEXCEPTION(rete, "CURLE", curl_easy_strerror(ret));

    d->handle = curl_multi_init();
    if (d->handle == 0)
        throw PROTOCOLEXCEPTION(CURLM_BAD_ALLOC, "CURLM", strerror(CURLM_BAD_ALLOC));
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
        throw PROTOCOLEXCEPTION(retm, "CURLM", curl_multi_strerror(retm));
}

const gchar* HttpProtocol::name()
{
    return "HTTP download";
}

bool nocaseCompare(gchar lhs, gchar rhs)
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

bool HttpProtocol::canProcess(const gchar *uri)
{
    // URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    // Common schemes include "file", "http", "svn+ssh", etc.

    gchar *scheme = strchr(uri, ':');
    if (equal(uri, scheme, "http", nocaseCompare) == 0)
        return true;

    return false;
}

void HttpProtocol::setOptions(const gchar *opts)
{
}

const gchar* HttpProtocol::getAllOptions()
{
    return "<sessionNumber default=\"5\"> />";
}

void getFileName(CURL *handle, HttpTaskInfo *info)
{
    gchar *p = info->info->url + strlen(info->info->url);
    bool findNonNameChar = false;
    --p;
    while ( !findNonNameChar && (p >= info->info->url) )
    {
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
    }
    ++p;
    if (outputName != NULL)
        delete [] outputName;
    outputName = strdup(p);
}

void initTaskInfo(HttpTaskInfo *info)
{
#define CHECK_CURLE(ret)                                                \
    {                                                                   \
        if (rete != CURL_OK)                                            \
            throw PROTOCOLEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

    CURL *handle = curl_easy_init();
    if (handle == NULL)
        throw PROTOCOLEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(handle, CURLOPT_URL, info->info->url);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle, CURLOPT_FILETIME, 1);
    CHECK_CURLE(rete);

    rete = curl_easy_perform(handle);
    CHECK_CURLE(rete);

    double length;
    rete = curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
    CHECK_CURLE(rete);
    info->info->totalSize = static_cast<size_t>(length);
    info->info->finishSize = 0;

    info->info->bitMap->bytesPerBit = DefaultBytePerBlock;
    info->info->bitMap->length =
        (info->info->totalSize + info->info->bitMap->bytesPerBit - 1) / info->info->bitMap->bytesPerBit;
    info->info->bitMap->map = new char[info->info->bitMap->length];
    memset(info->info->bitMap->map, 0, info->info->bitMap->length);

    long remoteTime;
    rete = curl_easy_getinfo(handle, CURLINFO_FILETIME, &remoteTime);
    CHECK_CURLE(rete);
    info->remoteTime = static_cast<time_t>(remoteTime);

    rete = curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &info->mimeType);
    CHECK_CURLE(rete);

    if ( (info->info->outputName == NULL) || (info->info->outputName[0] == '\0') )
        getFileName(handle, info);
    string output = info->info->outputPath;
    output.append(info->info->outputName);
    info->file = fopen(output.c_str(), "wb");
    if (file == NULL)
        throw PROTOCOLEXCEPTION(FAIL_OPEN_FILE, "HTTP", strerror(FAIL_OPEN_FILE));

    initFileSize(info->file, info->info->totalSize);

    curl_easy_cleanup(handle);

#undef CHECK_CURLE
}

size_t makeSession(HttpTaskInfo *info, size_t begin, size_t len)
{
#define CHECK_CURLE(ret)                                                \
    {                                                                   \
        if (rete != CURL_OK)                                            \
            throw PROTOCOLEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

    HttpSessionInfo *sinfo = new HttpSessionInfo;
    sinfo->writePos = begin;
    sinfo->length = len;
    sinfo->parent = info;

    sinfo->handle = curl_easy_init();
    if (sinfo->handle == NULL)
        throw PROTOCOLEXCEPTION(CURL_BAD_ALLOC, "CURL", strerror(CURL_BAD_ALLOC));

    rete = curl_easy_setopt(sinfo->handle, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(sinfo->handle, CURLOPT_WRITEDATA, sinfo);
    CHECK_CURLE(rete);

    char header[128] = {0};
    sprintf(header, "Range:bytes=%d-%d", begin, begin + len - 1);
    sinfo->slist = NULL;
    sinfo->slist = curl_slist_append(sinfo->slist, header);
    if (sinfo->slist == NULL)
        throw PROTOCOLEXCEPTION(CURL_SLIST_BAD_ALLOC, "CURL", strerror(CURL_SLIST_BAD_ALLOC));
    CURLcode rete = curl_easy_setopt(sinfo->handle, CURLOPT_HTTPHEADER, sinfo->slist);
    CHECK_CURLE(rete);

    CURLMcode retm = curl_multi_add_handle(d->handle, sinfo->hande);
    if (retm != CURLM_OK)
    {
        curl_slist_free_all(sinfo->slist);
        sinfo->slist = 0;
        throw PROTOCOLEXCEPTION(retm, "CURL", curl_multi_strerror(retm));
    }

    info->sessions.push_back(sinfo);

#undef CHECK_CURLE
}

TaskID HttpProtocolData::getNewID()
{
    struct IsGreater
    {
        IsGreater(TaskID id) : id_(id) {}
        bool operator()(TaskID id)
            {
                return id_ > id;
            }

        TaskID id_;
    };
    typedef vector<TaskID> IDs;
    IDs ids(tasks.size(), -1);

    // insert sort
    for (Tasks::iterator it = tasks.begin();
         it != tasks.end();
         ++it)
    {
        IDs::iterator insertPos = find_if(ids.begin(), ids.end(), IsGreater(it->first));
        IDs.insert(insertPos, it->first);
    }

    TaskID ret = 0;
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

void HttpProtocol::addTask(const TaskID id, TaskInfo *info)
{
    if (info == NULL)
        throw PROTOCOLEXCEPTION(NULL_INFO, "HTTP", strerror(NULL_INFO));

    HttpTaskInfo *httpInfo = new HttpTaskInfo();
    httpInfo->info = info;

    initTaskInfo(httpInfo);

    // get the informaion, now devide to sessions
    if (info->bitMap.bytesPerBit() == 0)
    {
        info->bitMap.setLength(info->total_size, bytesPerBit);
    }

    unsigned int sessionNumber = d->defaultSessionNumber;
    int begin = 0;
    int end = 0;
    int nextBegin = 0;
    for(int i=0; i<sessionNumber; ++i)
    {
        end = info->bitMap.find(true, begin);
        nextBegin = info->bitMap.find(false, end);
        if (nextBegin == info->bitMap.size())
        {
            // devide [begin, end) to sessionNumber-i directly.
            int n = sessionNumber - i;
            while (begin < end)
            {
                int len = (end - begin) / n * info->bitMap.bytesPerBit();
                begin += makeSession(httpInfo, begin, len);
                --n;
            }
        }
        else if (end == info->bitMap.size())
        {
            // devide [begin, end) to sessionNumber-i directly.
            int n = sessionNumber - i;
            while (begin < end)
            {
                int len = (end - begin) / n * info->bitMap.bytesPerBit();
                begin += makeSession(httpInfo, begin, len);
                --n;
            }
        }
        else
        {
            // make [begin, end) a seesion.
            makeSession(httpInfo, begin, (end - begin) * info->bitMap.bytesPerBit());
        }
        begin = nextBegin;
    }

    d->tasks.insert(Tasks::value_type(id, httpInfo));
}

void HttpProtocol::delTask(const TaskID id)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->delTask(it);
}

bool HttpProtocol::hasTask(const TaskID id)
{
    Tasks::iterator it = d->tasks.find(id);
    return it != d->tasks.end();
}

BitMap HttpProtocol::getTaskBitMap(const TaskID id)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    return it->second->info->bitMap;
}

void HttpProtocol::saveTask(std::ostream_iterator &out)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->saveTask(it, out);
}

void HttpProtocol::loadTask(const TaskID id,
                            std::istream_iterator &begin,
                            std::istream_iterator &end)
{
    Tasks::iterator it = d->tasks.find(id);
    if (it == d->tasks.end())
    {
        printf("id %d doesn't exist\n", id);
        return;
    }

    d->loadTask(it, begin, end);
}

size_t HttpProtocol::performDownload(size_t *downloaded)
{
    int running;

    downloadSize = 0;
    CURLMcode ret = CURLM_OK;
    while ( (ret = curl_multi_perform(d->handle, &running)) ==
             CURLM_CALL_MULTI_PERFORM);
    if (ret != CURLM_OK)
        throw PROTOCOLEXCEPTION(retm, "CURL", curl_multi_strerror(retm));

    if (running < d->tasks.size())
    {
        checkTasks();
    }

    *downloaded = downloadSize;
    return running;
}

size_t HttpProtocol::performUpload(size_t *uploaded)
{
    // http protocol won't upload data.
    *uploaded = 0;
    return 0;
}

void HttpProtocol::checkTasks()
{
    // check task status.
    throw PROTOCOLEXCEPTOIN(1, "HTTP", "not implement");
}
