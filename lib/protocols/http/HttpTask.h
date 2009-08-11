#ifndef HTTP_TASK_HEADER
#define HTTP_TASK_HEADER

#include <vector>
#include <string>

#include "lib/utility/FileManager.h"

#include "lib/protocols/TaskBase.h"
#include "lib/protocols/ProtocolBase.h"

#include "BitMap.h"
#include "HttpConfigure.h"

class HttpSession;

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
        FAIL_FILE_IO,
        XML_PARSE_ERROR,
        OTHER,
    };

    HttpTask();
    virtual ~HttpTask();

    virtual const char* uri();
    virtual const char* outputDir();
    virtual const char* outputName();
    virtual const char* options();
    virtual const char* mimeType();
    virtual const char* comment();
    virtual const char* notice();
    virtual size_t      totalSize();
    virtual size_t      downloadSize();
    virtual size_t      uploadSize();
    virtual int         totalSource();
    virtual int         validSource();
    virtual std::vector<bool> validBitmap();
    virtual std::vector<bool> downloadBitmap();
    virtual TaskState   state();
    virtual ProtocolBase *protocol();

    virtual void fdSet(fd_set* read, fd_set* write, fd_set* exc, int* max);
    virtual size_t performDownload();
    virtual size_t performUpload();

    virtual int error();
    virtual const char* strerror(int error);

    enum InternalState
    {
        HT_INVALID,
        HT_PREPARE,
        HT_DOWNLOAD,
        HT_DOWNLOAD_WITHOUT_LENGTH,
        HT_ERROR,
        HT_FINISH,
    };

    InternalState internalState();
    void setInternalState(InternalState state);
    void setError(Error error, const char* errstr = NULL);

    void initTask();
    void sessionFinish(HttpSession* ses);
    const HttpConfigure& configure();

    void seekFile(size_t pos);
    ssize_t writeFile(void *buffer, size_t size);

private:
    std::string uri_;
    std::string outputDir_;
    std::string outputName_;
    HttpConfigure config_;
    std::string mimeType_;
    std::string comment_;
    size_t totalSize_;
    size_t downloadSize_;
    int totalSource_;
    int validSource_;
    BitMap validBitmap_;
    BitMap downloadBitMap_;
    TaskState state_;
    ProtocolBase* base_;

    HttpConfigure conf;

    Utility::FileManager file_;
    std::vector<HttpSession> sessions_;
};

#endif
