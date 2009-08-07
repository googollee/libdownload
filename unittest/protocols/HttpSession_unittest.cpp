#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.h"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

std::string testUri;
HttpSession *ses = NULL;

HttpTask::HttpTask() {}
HttpTask::~HttpTask() {}
const char* HttpTask::getUri() { return testUri.c_str(); }
const char* HttpTask::getOutputDir() { return NULL; }
const char* HttpTask::getOutputName() { return NULL; }
const char* HttpTask::getOptions() { return NULL; }
const char* HttpTask::getMimeType() { return NULL; }
const char* HttpTask::getComment() { return NULL; }
size_t      HttpTask::getTotalSize() { return 0; }
size_t      HttpTask::getDownloadSize() { return 0; }
size_t      HttpTask::getUploadSize() { return 0; }
int         HttpTask::getTotalSource() { return 0; }
int         HttpTask::getValidSource() { return 0; }
std::vector<bool> HttpTask::getValidBitmap() { return std::vector<bool>(); }
std::vector<bool> HttpTask::getDownloadBitmap() { return std::vector<bool>(); }
ProtocolBase *HttpTask::getProtocol() { return NULL; }
void HttpTask::getFdSet(fd_set* /*read*/, fd_set* /*write*/, fd_set* /*exc*/, int* /*max*/) {}
size_t HttpTask::performDownload() { return 0; }
size_t HttpTask::performUpload() { return 0; }
const char *HttpTask::strerror(int error) { error = error; return NULL; }
TaskState HttpTask::getState() { return TASK_WAIT; }

int s = 0;

HttpTask::InternalState HttpTask::internalState()
{
    return (HttpTask::InternalState)s;
}

void HttpTask::initTask()
{
    file.open("./test.data", File::OF_Write | File::OF_Create);
    s = (int)HT_DOWNLOAD;

    CURL *ehandle = ses->handle();
    double length;
    curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

    if (length > 0)
        ses->setLength(long(length));
}

void HttpTask::sessionFinish(HttpSession *ses)
{
//     CURLcode rete = curl_easy_pause(ses->handle(), CURLPAUSE_ALL);
//     CHECK_CURLE(rete);

    file.close();
}

void HttpTask::seekFile(size_t pos)
{
    if (!file.seek(pos, File::SF_FromBegin))
        printf("seek fail\n");
}

size_t HttpTask::writeFile(void *buffer, size_t size)
{
    return file.write(buffer, size);
}

TEST(HttpSessionTest, Normal)
{
    testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
    s = 1;
    {
        File f;
        f.open("./test.data", File::OF_Write | File::OF_Create | File::OF_Truncate);
    }

    HttpTask task;
    HttpSession session(task);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, CantGetRightLenght)
{
    testUri = "http://www.boost.org/doc/libs/1_39_0/more/getting_started/unix-variants.html";
    s = 1;
    {
        File f;
        f.open("./test.data", File::OF_Write | File::OF_Create | File::OF_Truncate);
    }

    HttpTask task;
    HttpSession session(task);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, PartDownload)
{
    testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
    s = 1;
    {
        File f;
        f.open("./test.data", File::OF_Write | File::OF_Create | File::OF_Truncate);
    }

    HttpTask task;
    HttpSession session(task, 0, 100);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, PartDownloadFromMiddle)
{
    {
        File f;
        f.open("./test.data", File::OF_Write | File::OF_Create | File::OF_Truncate);
    }

    {
        testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
        s = 1;

        HttpTask task;
        HttpSession session(task, 0, 50);
        ses = &session;

        CURLcode ret = curl_easy_perform(session.handle());
        if (ret != CURLE_OK)
            printf("meet error: %s\n", curl_easy_strerror(ret));

        ses = NULL;
    }

    {
        testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
        s = 1;

        HttpTask task;
        HttpSession session(task, 50, 50);
        ses = &session;

        CURLcode ret = curl_easy_perform(session.handle());
        if (ret != CURLE_OK)
            printf("meet error: %s\n", curl_easy_strerror(ret));

        ses = NULL;
    }
}
