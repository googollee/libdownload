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

    HttpConfigure(const HttpConfigure& arg)
        : sessionNumber(arg.sessionNumber),
          minSessionBlocks(arg.minSessionBlocks),
          bytesPerBlock(arg.bytesPerBlock),
          referer(arg.referer),
          userAgent(arg.userAgent)
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
            }

            return *this;
        }
};

#endif
