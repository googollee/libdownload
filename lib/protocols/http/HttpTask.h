#ifndef HTTP_TASK_HEADER
#define HTTP_TASK_HEADER

#include "TaskBase.h"
#include "ProtocolBase.h"

#include "lib/protocols/http/BitMap.h"

#include <string>

template<typename File>
class HttpTask : public TaskBase<File>
{
public:
    HttpTask();
    virtual ~HttpTask();

    virtual const char *getUri();
    virtual const char *getOutputDir();
    virtual const char *getOutputName();
    virtual const char *getOptions();
    virtual const char *getMimeType();
    virtual const char *getComment();
    virtual size_t      getTotalSize();
    virtual size_t      getDownloadSize();
    virtual size_t      getUploadSize();
    virtual int         getTotalSource();
    virtual int         getValidSource();
    virtual std::vector<bool> getValidBitmap();
    virtual std::vector<bool> getDownloadBitmap();
    virtual TaskState   getState();
    virtual ProtocolBase *getProtocol();

    virtual void getFdSet(fd_set *read, fd_set *write, fd_set *exc, int *max);
    virtual size_t performDownload();
    virtual size_t performUpload();

    virtual const char *strerror(int error);

private:
    friend class HttpProtocol;

    std::string uri;
    std::string outputDir;
    std::string outputName;
    HttpConfigure config;
    std::string mimeType;
    std::string comment;
    size_t totalSize;
    size_t downloadSize;
    int totalSource;
    int validSource;
    BitMap validBitmap;
    BitMap downloadBitMap;
    TaskState state;
    ProtocolBase* base;
};

#endif
