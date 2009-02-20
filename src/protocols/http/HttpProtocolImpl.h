#ifndef HTTP_PROTOCOL_IMPL_HEADER
#define HTTP_PROTOCOL_IMPL_HEADER

#include "HttpProtocol.h"
#include "utility/File.h"

#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

static int DefaultSessionNumber = 5;
static int DefaultMinSessionBlocks = 200;
static int DefaultBytesPerBlock = 512;

struct HttpProtocolData;
struct HttpTask;
struct HttpSession;

typedef std::map<TaskId, HttpTask*> Tasks;
typedef std::vector<HttpSession*> Sessions;

struct HttpConfigure
{
    int sessionNumber;
    int minSessionBlocks;
    int bytesPerBlock;

    HttpConfigure()
        : sessionNumber(DefaultSessionNumber),
          minSessionBlocks(DefaultMinSessionBlocks),
          bytesPerBlock(DefaultBytesPerBlock)
        {}
};

enum HttpTaskState
{
    HT_INVALID,
    HT_PREPARE,
    HT_DOWNLOAD,
    HT_FINISH,
};

struct HttpSession
{
    CURL *handle;
    size_t pos;
    size_t length;
    HttpTask &t;

    HttpSession(HttpTask &task)
        : handle(NULL),
          pos(0),
          length(0),
          t(task)
        {}
};

struct HttpTask
{
    TaskInfo *info;
    filesystem::File file;
    HttpConfigure conf;
    HttpTaskState state;
    HttpProtocolData &d;

    Sessions sessions;

    HttpTask(HttpProtocolData &data)
        : info(NULL),
          state(HT_INVALID),
          d(data)
        {}
};

struct HttpProtocolData
{
    HttpProtocol *p;

    CURLM *handle;
    Tasks tasks;
    int running;
    Sessions finishSessions;

    void initTask(HttpTask *task);
    void removeTask(const Tasks::iterator &it);
    void saveTask(const Tasks::iterator &it,
                  std::ostream_iterator<char> &out);
    void loadTask(HttpTask *task,
                  std::istream_iterator<char> &begin,
                  std::istream_iterator<char> &end);
    void checkTask(HttpTask *task);
    void checkSession(HttpSession *session);
    void checkTasks();

    void makeSession(HttpTask *task, size_t pos, size_t len);
    void removeSession(HttpSession *session);

    HttpProtocolData()
        : p(NULL),
          handle(NULL),
          running(0)
        {}
};

#endif
