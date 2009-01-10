#ifndef HTTP_PROTOCOL_HEADER
#define HTTP_PROTOCOL_HEADER

#include "../ProtocolBash.h"

#include <memory>

struct HttpProtocolData;

class HttpProtocol : public ProtocolBase
{
public:
    HttpProtocol();
    virtual ~HttpProtocol();

    virtual const gchar* name();
    virtual bool canProcess(const gchar *uri);
    virtual void setOptions(const gchar *opts);
    virtual const gchar* getAllOptions();

    virtual void addTask(const TaskID id, TaskInfo *info);
    virtual void delTask(const TaskID id);
    virtual bool hasTask(const TaskID id);
    virtual BitMap getTaskBitmap(const TaskID id);

    virtual void saveTask(std::ostream_iterator &out);
    virtual void loadTask(const TaskID id,
                          std::istream_iterator &begin,
                          std::istream_iterator &end);

//     virtual void getFdSet(fd_set *read_fd_set,
//                           fd_set *write_fd_set,
//                           fd_set *exc_fd_set,
//                           int *max_fd) = 0;
    virtual size_t performDownload(size_t *downloaded);
    virtual size_t performUpload(size_t *uploaded);

private:
    std::auto_ptr<HttpProtocolData> d;

    enum HttpError
    {
        CURL_BAD_ALLOC,
        CURL_SLIST_BAD_ALLOC,
        CURLM_BAD_ALLOC,
        NULL_INFO,
        BAD_FILE_LENGTH,
    };

    static const char* strerror(HttpError error);
    void checkTasks();
};

#endif
