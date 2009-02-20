#include "protocols/http/HttpProtocol.h"

#include <utility/DownloadException.h>

#include <gtest/gtest.h>

void ErrorCallback(TaskInfo *info, int error)
{
    printf("%s task %d error: (%d)%s\n", info->protocol->name(), info->id, error, info->protocol->strerror(error));
}

void DownloadFinishCallback(TaskInfo *info)
{
    printf("%s task %d download finished\n", info->protocol->name(), info->id);
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
        info->url = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
        info->outputPath = "./";
        info->outputName = NULL;

        http.addTask(info);
        info->state = DOWNLOAD;

        while (http.perform() > 0)
        {
//            printf("progress: %g\n", (double)info->downloadSize / info->totalSize);
        }

        delete info;
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
    }
}
