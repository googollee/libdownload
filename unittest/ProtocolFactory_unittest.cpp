#include "ProtocolFactory.h"
#include "protocols/HttpProtocol.h"

#include <gtest/gtest.h>

TEST(ProtocolFactory, NormalTest)
{
    ProtocolFactory factory;
    HttpProtocol *http = new HttpProtocol;
    EXPECT_EQ(factory.addProtocol(std::auto_ptr<ProtocolBase>(http)), true);
    EXPECT_EQ(factory.addProtocol(std::auto_ptr<ProtocolBase>(http)), false);

    EXPECT_EQ(factory.getProtocol("http://curl.haxx.se/libcurl/c/curl_easy_setopt.html"),
              http);
    EXPECT_EQ(factory.getProtocol("ftp://curl.haxx.se/libcurl/c/curl_easy_setopt.html"),
              (ProtocolBase*)NULL);
}
