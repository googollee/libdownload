#ifndef HTTP_PROTOCOL_IMPL_HEADER
#define HTTP_PROTOCOL_IMPL_HEADER

#include "HttpProtocol.h"
#include "../../utility/File.h"

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
    slist *header;
    size_t writePos;
    size_t length;
    HttpTaskInfo *parent;
};

typedef vector<HttpSessionInfo*> Sessions;

struct HttpTaskInfo
{
    TaskInfo *info;
    File file;
    time_t remoteTime;
    std::string mimeType;
    time_t lastRunTime;

    Sessions sessions;

    HttpTaskInfo()
        : info(0),
          file(0),
          remoteTime(-1),
          mimeType(0),
          lastRunTime(-1)
        {}
};

typedef map<TaskID, HttpTaskInfo*> Tasks;

struct HttpProtocolData
{
    CURLM *handle;
    Tasks tasks;
    unsigned int defaultSessionNumber;

    size_t sessionsPerTask;
    void delTask(const Tasks::iterator &it);

    void saveTask(const Tasks::iterator &it,
                  std::ostream_iterator &out);
    void loadTask(const Tasks::iterator &it,
                  std::istream_iterator &begin,
                  std::istream_iterator &end);

    TaskID getNewID();
};

#endif
