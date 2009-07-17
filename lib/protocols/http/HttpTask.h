#ifndef HTTP_TASK_HEADER
#define HTTP_TASK_HEADER

#include "lib/protocols/TaskBase.h"
#include "lib/protocols/ProtocolBase.h"

#include "BitMap.h"
#include "HttpConfigure.h"

#include <string>

template <typename File>
class HttpSession;

template <typename File>
class HttpTask : public TaskBase
{
public:
    enum Error
    {
        CURL_BAD_ALLOC,
        CURLM_BAD_ALLOC,
        NULL_INFO,
        BAD_FILE_LENGTH,
        FAIL_OPEN_FILE,
        TASK_EXIST,
        TASK_NOT_EXIST,
        XML_PARSE_ERROR,
        LAST_ERROR = XML_PARSE_ERROR,
    };

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
    friend class HttpSession<File>;

    enum InternalState
    {
        HT_INVALID,
        HT_PREPARE,
        HT_DOWNLOAD,
        HT_PART_ERROR,
        HT_ERROR,
        HT_FINISH,
    };

    void initTask();
    InternalState internalState();
    void sessionFinish(HttpSession<File> *ses);
    void seekFile(size_t pos);
    size_t writeFile(void *buffer, size_t size);

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

    File file;
};

#endif
