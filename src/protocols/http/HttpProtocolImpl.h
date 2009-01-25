#ifndef HTTP_PROTOCOL_IMPL_HEADER
#define HTTP_PROTOCOL_IMPL_HEADER

#include "HttpProtocol.h"
#include <utility/File.h>

#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

const unsigned int DefaultSessionNumber = 5;
const size_t DefaultBytePerBlock = 512;

struct HttpTaskInfo;

struct HttpSessionInfo
{
    CURL *handle;
    size_t writePos;
    size_t length;
    HttpTaskInfo *parent;

    HttpSessionInfo()
        : handle(NULL),
          writePos(0),
          length(0),
          parent(NULL)
        {}
};

typedef std::vector<HttpSessionInfo*> Sessions;

struct HttpTaskInfo
{
    TaskInfo *taskInfo;
    filesystem::File *file;
    time_t remoteTime;
    std::string mimeType;
    time_t lastRunTime;

    Sessions sessions;

    HttpTaskInfo()
        : taskInfo(NULL),
          file(NULL),
          remoteTime(0),
          lastRunTime(0)
        {}
};

typedef std::map<TaskId, HttpTaskInfo*> Tasks;

struct HttpProtocolData
{
    CURLM *handle;
    Tasks tasks;
    unsigned int defaultSessionNumber;
    int running;

    void delTask(const Tasks::iterator &it);

    void saveTask(const Tasks::iterator &it,
                  std::ostream_iterator<char> &out);
    void loadTask(const Tasks::iterator &it,
                  std::istream_iterator<char> &begin,
                  std::istream_iterator<char> &end);

    TaskId getNewID();

    void makeSession(HttpTaskInfo *info, size_t being, size_t len);

    HttpProtocolData()
        : handle(NULL),
          defaultSessionNumber(DefaultSessionNumber),
          running(-1)
        {}
};

#endif
