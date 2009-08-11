#ifndef HTTP_CONFIGURE_CLASS_HEAD
#define HTTP_CONFIGURE_CLASS_HEAD

#include <string>

struct HttpConfigure
{
    int sessionNumber;
    int minSessionBlocks;
    int bytesPerBlock;
    std::string referer;
    std::string userAgent;
    int retryCount;
    long connectingTimeout;

    HttpConfigure()
        : sessionNumber(5),
          minSessionBlocks(100),
          bytesPerBlock(512),
          referer(""),
          userAgent(""),
          retryCount(-1),
          connectingTimeout(-1)
        {}

    HttpConfigure(const HttpConfigure& arg)
        : sessionNumber(arg.sessionNumber),
          minSessionBlocks(arg.minSessionBlocks),
          bytesPerBlock(arg.bytesPerBlock),
          referer(arg.referer),
          userAgent(arg.userAgent),
          retryCount(arg.retryCount),
          connectingTimeout(arg.connectingTimeout)
        {}

    const HttpConfigure& operator=(const HttpConfigure& arg)
        {
            if (this != &arg)
            {
                sessionNumber = arg.sessionNumber;
                minSessionBlocks = arg.minSessionBlocks;
                bytesPerBlock = arg.bytesPerBlock;
                referer = arg.referer;
                userAgent = arg.userAgent;
                retryCount = arg.retryCount;
                connectingTimeout = arg.connectingTimeout;
            }

            return *this;
        }
};

#endif
