#ifndef HTTP_PROTOCOL_HEADER
#define HTTP_PROTOCOL_HEADER

#include "protocols/ProtocolBase.h"
#include "HttpConfigure.h"

class HttpProtocol : public ProtocolBase
{
public:
    HttpProtocol();
    virtual ~HttpProtocol();

    virtual const char* name();

    virtual const char* getOptionsDetail();
    virtual const char* getOptions();
    virtual void setOptions(const char *options);

    virtual bool canProcess(const char *uri);
    virtual const char* getTaskOptions(const char *uri);
    template <typename File>
    virtual auto_ptr<TaskBase<File> > getTask(const char *uri,
                                              const char *outputDir,
                                              const char *outputName,
                                              const char *options,
                                              const char *comment);

private:
    HttpConfigure defaultConf;
};

#endif
