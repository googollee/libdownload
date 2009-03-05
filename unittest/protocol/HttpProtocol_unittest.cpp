#include "protocols/http/HttpProtocol.h"

#include <utility/DownloadException.h>

#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

void ErrorCallback(TaskInfo *info, int error)
{
    printf("%s task %d error: (%d)%s\n", info->protocol->name(), info->id, error, info->protocol->strerror(error));
}

void DownloadFinishCallback(TaskInfo *info)
{
    ProtocolBase *p = info->protocol;

    printf("%s task %d download finished\n", p->name(), info->id);

//     std::ostringstream out;
//     p->saveTask(info->id, out);
//     std::string str = out.str();

//     std::cout << str << std::endl;
//     std::istringstream in(str);
//     TaskInfo *i = new TaskInfo;
//     i->id = 1;
//     p->loadTask(i, in);
}

void LogTaskInfoCallback(TaskInfo *info, const char *log)
{
    printf("%s task %d: %s\n", info->protocol->name(), info->id, log);
}

void LogProtocolInfoCallback(ProtocolBase *p, const char *log)
{
    printf("%s: %s\n", p->name(), log);
}

TEST(HttpTest, NormalDownload)
{
    try
    {
        HttpProtocol http;

        http.taskError.connect(ErrorCallback);
        http.downloadFinish.connect(DownloadFinishCallback);
        http.taskLog.connect(LogTaskInfoCallback);
        http.protocolLog.connect(LogProtocolInfoCallback);

        TaskInfo *info = new TaskInfo;

        info->id = 0;
        info->uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = NULL;

        http.addTask(info);
        info->state = DOWNLOAD;

        while (http.perform() > 0)
        {}

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
    }
}
