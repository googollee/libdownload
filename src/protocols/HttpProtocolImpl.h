#ifndef HTTP_PROTOCOL_IMPL_HEADER
#define HTTP_PROTOCOL_IMPL_HEADER

#include "HttpProtocol.h"

#include <utility/File.h>
#include <utility/SimpleXmlParser.h>

#include <curl/curl.h>

#include <map>
#include <vector>
#include <string>

struct HttpProtocolData;
struct HttpTask;
struct HttpSession;

typedef std::map<TaskInfo*, HttpTask*> Tasks;
typedef std::vector<HttpSession*> Sessions;

struct HttpConfigure
{
    int sessionNumber;
    int minSessionBlocks;
    int bytesPerBlock;
    std::string referer;
    std::string userAgent;

    HttpConfigure()
        : sessionNumber(5),
          minSessionBlocks(100),
          bytesPerBlock(512),
          referer(""),
          userAgent("")
        {}
};

struct HttpProtocolData
{
    HttpProtocol *p; // a reference

    CURLM *handle;
    HttpConfigure defaultConf;
    Tasks tasks;
    int running;
    Sessions finishSessions;
    size_t performSize;

    void initTask(HttpTask *task);
    void removeTask(const Tasks::iterator &it);

    void saveTask(const Tasks::iterator &it, std::string &data);
    void loadTask(HttpTask *task, std::string &data);

    void checkTask(HttpTask *task);
    void checkSession(HttpSession *session);
    void checkTasks();

    void makeSession(HttpTask *task, size_t pos, size_t len);
    bool splitMaxSession(HttpTask *task);
    void removeSession(HttpSession *session);

    HttpProtocolData()
        : p(NULL),
          handle(NULL),
          running(0),
          performSize(0)
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
    HttpTask *t; // a reference.

    CURL *handle;
    size_t pos;
    int length;  // -1 mean unknow length.

    HttpSession(HttpTask *task)
        : t(task),
          handle(NULL),
          pos(0),
          length(-1)
        {}
};

struct HttpTask
{
    HttpProtocolData *d; // a reference

    TaskInfo *info;
    File file;
    HttpConfigure conf;
    HttpTaskState state;

    Sessions sessions;

    HttpTask(HttpProtocolData *data)
        : d(data),
          info(NULL),
          conf(data->defaultConf),
          state(HT_INVALID)
        {}
};

class HttpConfXmlParser : public SimpleXmlParser
{
private:
    void text(const char *text,
              size_t textLen)
        {
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
                conf.sessionNumber = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "MinSessionBlocks") == 0)
            {
                conf.minSessionBlocks = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "BytesPerBlock") == 0)
            {
                conf.bytesPerBlock = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "TotalSize") == 0)
            {
                totalSize = atoi(data.c_str());
            }
            else if (strcmp(getElement(), "BitMap") == 0)
            {
                downloadMap = BitMap(totalSize, conf.bytesPerBlock);
                size_t finishSize = 0;
                for (int i=0, n=data.length()-1; i<n; ++i)
                {
                    if (data[i] == '1')
                    {
                        downloadMap.set(i, true);
                        finishSize += conf.bytesPerBlock;
                    }
                    else
                    {
                        downloadMap.set(i, false);
                    }
                }
                int i = data.length() - 1;
                if (data[i] == '1')
                {
                    // need calculate last block seperately.
                    downloadMap.set(i, true);
                    finishSize += totalSize - (i * conf.bytesPerBlock);
                }
                else
                {
                    downloadMap.set(i, false);
                }
                downloadSize = finishSize;
            }
            else
            {
                LOG(0, "can't handle <%s>%s</%s>\n", getElement(), data.c_str(), getElement());
            }
        }

    HttpTask *task_;

public:
    HttpConfXmlParser()
        : totalSize(0),
          downloadSize(0)
        {
            memset(&conf, sizeof(conf), 0);
        }

    HttpConfigure conf;
    size_t totalSize;
    size_t downloadSize;
    BitMap downloadMap;
};

#endif
