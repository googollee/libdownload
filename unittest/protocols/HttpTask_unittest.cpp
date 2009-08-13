#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.h"

#include <gtest/gtest.h>

#include <string>

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
    HttpTaskUnitTest::setOutput(task, "./", "test.data");

    task.start();

    while ( (task.state() != TaskBase::TASK_FINISH) &&
            (task.state() != TaskBase::TASK_ERROR) )
    {
        int down = task.performDownload();
        printf("download %d\r", down);
    }

    printf("state: %d\n", task.state());
}
