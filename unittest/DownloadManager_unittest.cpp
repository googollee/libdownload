#include "DownloadManager.h"
#include "ProtocolFactory.h"
#include "protocols/HttpProtocol.h"

#include <gtest/gtest.h>

TEST(DownloadManager, NormalTest)
{
    std::auto_ptr<ProtocolFactory> factory(new ProtocolFactory());
    factory->addProtocol(std::auto_ptr<ProtocolBase>(new HttpProtocol));

    DownloadManager manager(factory);
    const char *uri = "http://curl.haxx.se/libcurl/c/curl_easy_setopt.html";
    std::string taskOptions = manager.getTaskOptions(uri);
    std::cout << "task options:\n" << taskOptions << std::endl;
    Task task = manager.addTask(uri,
                                "./",
                                NULL,
                                taskOptions.c_str(),
                                NULL);
    manager.startTask(task);

    while (manager.perform() > 0)
    {
        if (task.totalSize() > 0)
            printf("progress: %lu%%, %d/%d\r",
                   task.downloadSize() * 100 / task.totalSize(),
                   task.validSource(),
                   task.totalSource()
                );
    }

    manager.removeTask(task);
}
