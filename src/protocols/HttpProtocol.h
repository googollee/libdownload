#ifndef HTTP_PROTOCOL_HEADER
#define HTTP_PROTOCOL_HEADER

#include "ProtocolBase.h"

#include <memory>

struct HttpProtocolData;

class HttpProtocol : public ProtocolBase
{
public:
    HttpProtocol();
    virtual ~HttpProtocol();

    virtual const char* name();
    virtual const char* getOptionsDetail();

    virtual const char* getOptions();
    virtual void        setOptions(const char* options);

    virtual bool        canProcess(const char *uri);
    virtual const char* getTaskOptions(const char *uri);

    virtual void        addTask     (TaskInfo *info);
    virtual void        removeTask  (TaskInfo *info);
    virtual bool        hasTask     (TaskInfo *info);
    virtual void        flushTask   (TaskInfo *info);
    virtual const char* getTaskState(TaskInfo *info);

    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int    *max_fd);
    virtual int perform();

    virtual const char* strerror(int error);

private:
    std::auto_ptr<HttpProtocolData> d;

};

#endif
