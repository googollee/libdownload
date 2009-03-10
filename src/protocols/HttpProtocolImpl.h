#ifndef HTTP_PROTOCOL_IMPL_HEADER
#define HTTP_PROTOCOL_IMPL_HEADER

#include "HttpProtocol.h"
#include "utility/File.h"

#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

struct HttpProtocolData;
struct HttpTask;
struct HttpSession;

typedef std::map<TaskId, HttpTask*> Tasks;
typedef std::vector<HttpSession*> Sessions;

struct HttpConfigure
{
    static int DefaultSessionNumber;
    static int DefaultMinSessionBlocks;
    static int DefaultBytesPerBlock;

    int sessionNumber;
    int minSessionBlocks;
    int bytesPerBlock;

    HttpConfigure()
        : sessionNumber(DefaultSessionNumber),
          minSessionBlocks(DefaultMinSessionBlocks),
          bytesPerBlock(DefaultBytesPerBlock)
        {}
};

int HttpConfigure::DefaultSessionNumber = 5;
int HttpConfigure::DefaultMinSessionBlocks = 100;
int HttpConfigure::DefaultBytesPerBlock = 512;

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
    int length;  // -1 mean unknow length.
    HttpTask *t; // a reference.

    HttpSession(HttpTask *task)
        : handle(NULL),
          pos(0),
          length(-1),
          t(task)
        {}
};

struct HttpTask
{
    TaskInfo *info;
    filesystem::File file;
    HttpConfigure conf;
    HttpTaskState state;
    HttpProtocolData *d; // a reference

    Sessions sessions;

    HttpTask(HttpProtocolData *data)
        : info(NULL),
          state(HT_INVALID),
          d(data)
        {}
};

struct HttpProtocolData
{
    HttpProtocol *p; // a reference

    CURLM *handle;
    Tasks tasks;
    int running;
    Sessions finishSessions;

    void initTask(HttpTask *task);
    void removeTask(const Tasks::iterator &it);

    void saveTask(const Tasks::iterator &it, std::ostream &out);
    void loadTask(HttpTask *task, std::istream &in);

    void checkTask(HttpTask *task);
    void checkSession(HttpSession *session);
    void checkTasks();

    void makeSession(HttpTask *task, size_t pos, size_t len);
    bool splitMaxSession(HttpTask *task);
    void removeSession(HttpSession *session);

    HttpProtocolData()
        : p(NULL),
          handle(NULL),
          running(0)
        {}
};

#endif
