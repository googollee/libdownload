#include "protocols/http/HttpProtocol.h"

#include <utility/DownloadException.h>

#include <gtest/gtest.h>

TEST(HttpTest, NormalDownload)
{
    try
    {
        HttpProtocol http;
        TaskInfo *info = new TaskInfo;

        info->url = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = NULL;

        http.addTask(0, info);

        size_t download;
        size_t upload;
        while (http.perform(&download, &upload) > 0);

        http.delTask(0);

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
    }
}

