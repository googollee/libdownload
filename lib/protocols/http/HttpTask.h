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

    virtual const char* uri()                  { return uri_.c_str(); }
    virtual const char* outputDir()            { return outputDir_.c_str(); }
    virtual const char* outputName()           { return outputName_.c_str(); }
    virtual const char* options();
    virtual const char* mimeType()             { return mimeType_.c_str(); }
    virtual const char* comment()              { return comment_.c_str(); }
    virtual const char* notice()               { return notice_.c_str(); }
    virtual size_t      totalSize()            { return totalSize_; }
    virtual size_t      downloadSize()         { return downloadSize_; }
    virtual size_t      uploadSize()           { return 0; }
    virtual int         totalSource()          { return totalSource_; }
    virtual int         validSource()          { return validSource_; }
    virtual std::vector<bool> validBitmap()    { return validBitmap_.getVector(); }
    virtual std::vector<bool> downloadBitmap() { return downloadBitmap_.getVector(); }
    virtual TaskState   state()                { return state_; }
    virtual ProtocolBase *protocol()           { return protocol_; }

    virtual bool start();
    virtual bool stop();

    virtual bool fdSet(fd_set* read, fd_set* write, fd_set* exc, int* max);
    virtual size_t performDownload();
    virtual size_t performUpload();

    virtual int error()                        { return err_; }
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

    InternalState internalState()              { return internalState_; }
    void setInternalState(InternalState state);
    void setError(Error error, const char* errstr = NULL);

    void initTask();
    void sessionFinish(HttpSession* ses);
    const HttpConfigure& configure()           { return config_; }

    bool writeFile(size_t pos, void *buffer, size_t size);

private:
    friend class HttpProtocol;
    friend struct HttpTaskUnitTest;

    void separateSession();
    void hasSessionFinish();
    bool checkFinish();
    void clearSessions();

    std::string uri_;
    std::string outputDir_;
    std::string outputName_;
    HttpConfigure config_;
    std::string mimeType_;
    std::string comment_;
    std::string notice_;
    size_t totalSize_;
    size_t downloadSize_;
    int totalSource_;
    int validSource_;
    BitMap validBitmap_;
    BitMap downloadBitmap_;
    TaskState state_;
    ProtocolBase* protocol_;

    Error err_;
    std::string errstr_;

    InternalState internalState_;

    CURLM* handle_;
    Utility::FileManager file_;
    typedef std::vector<HttpSession*> Sessions;
    Sessions sessions_;
    Sessions finishedSessions_;

    size_t writeLength_;
    int lastRunningHandle_;
};

#endif
