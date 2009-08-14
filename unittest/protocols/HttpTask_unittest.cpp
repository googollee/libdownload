#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

struct HttpTaskUnitTest
{
    static void setUri(HttpTask& task, const char* uri) { task.uri_ = uri; }
    static void setOutput(HttpTask& task, const char* path, const char* name)
        {
            if (path != NULL)
                task.outputDir_ = path;
            else
                task.outputDir_ = "";
            if (name != NULL)
                task.outputName_ = name;
            else
                task.outputName_ = "";
        }
};

TEST(HttpTaskTest, Normal)
{
    HttpTask task;
    HttpTaskUnitTest::setUri(task, "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html");
    HttpTaskUnitTest::setOutput(task, "./", "normal.download");

    task.start();

    while ( (task.state() != TaskBase::TASK_FINISH) &&
            (task.state() != TaskBase::TASK_ERROR) )
    {
        int down = task.performDownload();
        std::vector<bool> map = task.downloadBitmap();
        for (int i=0, n=map.size(); i<n; ++i)
        {
            if (map[i])
                printf("1");
            else
                printf("0");
        }
        printf("download %d\r", down);
    }

    printf("state: %d\n", task.state());
}
