#ifndef HTTP_PROTOCOL_HEADER
#define HTTP_PROTOCOL_HEADER

#include "../ProtocolBase.h"

#include <memory>

struct HttpProtocolData;

class HttpProtocol : public ProtocolBase
{
public:
    HttpProtocol();
    virtual ~HttpProtocol();

    virtual const char* name();
    virtual bool canProcess(const char *uri);

    virtual void loadOptions(std::istream &in);
    virtual void saveOptions(std::ostream &out);

    virtual const char* getAllOptions();
    virtual const char* control(const char* cmd);

    virtual void addTask(TaskInfo *info);
    virtual void removeTask(const TaskId id);
    virtual bool hasTask(const TaskId id);
    virtual const char *controlTask(const TaskId id, const char *cmd);

    virtual void loadTask(TaskInfo *info, std::istream &in);
    virtual void saveTask(const TaskId id, std::ostream &out);

    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int *max_fd);
    virtual int perform();

    virtual const char* strerror(int error);

private:
    std::auto_ptr<HttpProtocolData> d;

};

#endif
