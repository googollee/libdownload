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
    virtual void setOptions(const char *opts);
    virtual const char* getAllOptions();

    virtual void addTask(TaskInfo *info);
    virtual void removeTask(const TaskId id);
    virtual bool hasTask(const TaskId id);

    virtual void saveTask(const TaskId id,
                          std::ostream_iterator<char> &out);
    virtual void loadTask(TaskInfo *info,
                          std::istream_iterator<char> &begin,
                          std::istream_iterator<char> &end);

    virtual void getFdSet(fd_set *read_fd_set,
                          fd_set *write_fd_set,
                          fd_set *exc_fd_set,
                          int *max_fd);
    virtual size_t perform();

    virtual const char* strerror(int error);

private:
    std::auto_ptr<HttpProtocolData> d;

};

#endif
