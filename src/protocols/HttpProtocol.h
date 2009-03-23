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
    virtual bool        canProcess(const char *uri);
    virtual const char* getOptionsDetail();
    virtual const char* getTaskOptions(const char *uri);

    virtual void loadOptions(std::istream &in);
    virtual void saveOptions(std::ostream &out);

    virtual const char* getAllOptions();
    virtual void        control(ControlFlag f, const char* key, void *value);

    virtual void addTask      (TaskInfo *info);
    virtual void flushTask    (TaskInfo *info);
    virtual void removeTask   (TaskInfo *info);
    virtual bool hasTask      (TaskInfo *info);
    virtual void controlTask  (TaskInfo *info, ControlFlag f, const char* key, void *value);

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
