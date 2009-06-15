#include "protocols/HttpProtocol.h"

#include <utility/DownloadException.h>

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

void ErrorCallback(ProtocolBase *p, TaskInfo *info, int error)
{
    printf("in error callback: %p\n", p);
    printf("task %p error: %s\n", info, p->strerror(error));
}

void DownloadFinishCallback(TaskInfo *info)
{
    printf("%s task %p download finished\n", info->protocol->name(), info);
}

void LogTaskInfoCallback(TaskInfo *info, const char *log)
{
    printf("%s task %p: %s\n", info->protocol->name(), info, log);
}

void LogProtocolInfoCallback(ProtocolBase *p, const char *log)
{
    printf("%s: %s\n", p->name(), log);
}

TEST(HttpTest, CanProcessTest)
{
    HttpProtocol http;
    EXPECT_EQ(http.canProcess("http://xxx"), true);
    EXPECT_EQ(http.canProcess("https://xxx"), false);
    EXPECT_EQ(http.canProcess("ftp://xxx"), false);
    EXPECT_EQ(http.canProcess("someother://xxx"), false);
}

TEST(HttpTest, SetGetGlobalConf)
{
    HttpProtocol http;

    std::string data = "<SessionNumber>0</SessionNumber>";
    http.setOptions(data.c_str());
    std::string result = http.getOptions();
    EXPECT_EQ(result, "<SessionNumber>5</SessionNumber>");

    data = "<SessionNumber>100</SessionNumber>";
    http.setOptions(data.c_str());
    result = http.getOptions();
    EXPECT_EQ(result, data);
}

TEST(HttpTest, AddTask)
{
    try
    {
        HttpProtocol http;

        http.protocolLog.connect(LogProtocolInfoCallback);

        TaskInfo *info = new TaskInfo;

        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = "";
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        http.removeTask(info);

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }
}

TEST(HttpTest, AddTaskWithOptions)
{
    try
    {
        HttpProtocol http;

        http.protocolLog.connect(LogProtocolInfoCallback);

        TaskInfo *info = new TaskInfo;

        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = "";
        info->options = "<SessionNumber>100</SessionNumber>\n"
            "<BytesPerBlock>1024</BytesPerBlock>"
            "<MinSessionBlocks>200</MinSessionBlocks>\n";
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        http.flushTask(info);
        EXPECT_EQ(info->processData, "");

        http.removeTask(info);

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }
}

TEST(HttpTest, NormalDownload)
{
    try
    {
        HttpProtocol http;

        http.protocolLog.connect(LogProtocolInfoCallback);

        TaskInfo *info = new TaskInfo;

        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = "";
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        size_t size = 0;
        while (http.performDownload(&size) > 0)
        {
            if (info->totalSize > 0)
                printf("progress: %lu%%, %d/%d, download: %lu bytes\r",
                       info->downloadSize * 100 / info->totalSize,
                       info->validSource,
                       info->totalSource,
                       size
                    );
        }

        printf("download finish, total size = %lu\n", info->totalSize);
        printf("download finish, download size = %lu\n", info->downloadSize);
        printf("download map:\n");
        for (size_t i=0; i<info->downloadMap.size(); ++i)
        {
            printf("%d", info->downloadMap.get(i));
        }
        printf("\n");

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }
}

TEST(HttpTest, SaveLoadDownload)
{
    std::string data;
    HttpProtocol http;

    http.protocolLog.connect(LogProtocolInfoCallback);

    try
    {
        TaskInfo *info = new TaskInfo;

        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = "";
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        int i = 0;
        size_t size = 0;
        while (http.performDownload(&size) > 0)
        {
            if (info->totalSize > 0)
                printf("progress: %lu%%, %d/%d, download: %lu bytes\r",
                       info->downloadSize * 100 / info->totalSize,
                       info->validSource,
                       info->totalSource,
                       size
                    );
            if (i>3) break;
            if ( (info->totalSize > 0) &&
                 (info->downloadMap.get(20)) )
                ++i;
        }

        http.flushTask(info);
        data = info->processData.c_str();
        printf("%s\n", data.c_str());

        http.removeTask(info);

        delete info;

    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }

    try
    {
        TaskInfo *info = new TaskInfo;

        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = "";
        info->processData = data;
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        size_t size = 0;
        while (http.performDownload(&size) > 0)
        {
            if (info->totalSize > 0)
                printf("progress: %lu%%, %d/%d, download: %lu bytes\r",
                       info->downloadSize * 100 / info->totalSize,
                       info->validSource,
                       info->totalSource,
                       size
                    );
        }

        printf("download finish, total size = %lu\n", info->totalSize);
        printf("download finish, download size = %lu\n", info->downloadSize);
        printf("download map:\n");
        for (size_t i=0; i<info->downloadMap.size(); ++i)
        {
            printf("%d", info->downloadMap.get(i));
        }
        printf("\n");

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }
}

TEST(HttpTest, DownloadZeroFile)
{
    try
    {
        HttpProtocol http;

        http.protocolLog.connect(LogProtocolInfoCallback);

        TaskInfo *info = new TaskInfo;

        info->uri = "http://www.googollee.net/zero.file";
        info->outputPath = "./";
        info->outputName = "";
        info->taskError.connect(ErrorCallback);
        info->downloadFinish.connect(DownloadFinishCallback);
        info->taskLog.connect(LogTaskInfoCallback);

        http.addTask(info);

        size_t size = 0;
        while (http.performDownload(&size) > 0)
        {
            if (info->totalSize > 0)
                printf("progress: %lu%%, %d/%d, download: %lu bytes\r",
                       info->downloadSize * 100 / info->totalSize,
                       info->validSource,
                       info->totalSource,
                       size
                    );
        }

        printf("download finish, total size = %lu\n", info->totalSize);
        printf("download finish, download size = %lu\n", info->downloadSize);
        printf("download map:\n");
        for (size_t i=0; i<info->downloadMap.size(); ++i)
        {
            printf("%d", info->downloadMap.get(i));
        }
        printf("\n");

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }
}
