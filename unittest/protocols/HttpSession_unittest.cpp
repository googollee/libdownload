#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.h"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

std::string testUri;
HttpSession *ses = NULL;
HttpConfigure conf;

HttpTask::HttpTask() {}
HttpTask::~HttpTask() {}
const char* HttpTask::options() { return NULL; }
void HttpTask::fdSet(fd_set* /*read*/, fd_set* /*write*/, fd_set* /*exc*/, int* /*max*/) {}
size_t HttpTask::performDownload() { return 0; }
size_t HttpTask::performUpload() { return 0; }
const char* HttpTask::strerror(int error) { error = error; return NULL; }

void HttpTask::setInternalState(InternalState state)
{
    internalState_ = state;
}

void HttpTask::setError(Error /*error*/, const char* errstr)
{
    printf("error: %s\n", errstr);
}

std::string filename;

void HttpTask::initTask()
{
    file_.open(filename.c_str(), File::OF_Write | File::OF_Create);
    internalState_ = HT_DOWNLOAD;

    CURL *ehandle = ses->handle();
    double length;
    curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

    if (length > 0)
        ses->setLength(long(length));
    else
        internalState_ = HT_DOWNLOAD_WITHOUT_LENGTH;
}

void HttpTask::sessionFinish(HttpSession* /*ses*/)
{
//     CURLcode rete = curl_easy_pause(ses->handle(), CURLPAUSE_ALL);
//     CHECK_CURLE(rete);

    internalState_ = HT_FINISH;

    file_ .close();
}

void HttpTask::seekFile(size_t pos)
{
    if (!file_.seek(pos, File::SF_FromBegin))
    {
        internalState_ = HT_ERROR;
        printf("seek fail\n");
    }
}

ssize_t HttpTask::writeFile(void* buffer, size_t size)
{
    ssize_t ret = file_.write(buffer, size);
    if (ret == -1)
    {
        internalState_ = HT_ERROR;
    }

    return ret;
}

struct HttpTaskUnitTest
{
    static void setUri(HttpTask& task, const char* uri) { task.uri_ = uri; }
    static void setState(HttpTask& task, HttpTask::InternalState s) { task.internalState_ = s; }
};

TEST(HttpSessionTest, Normal)
{
    HttpTask task;
    HttpTaskUnitTest::setUri(task, "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html");
    HttpTaskUnitTest::setState(task, HttpTask::HT_PREPARE);

    filename = "./normal.download";
    HttpSession session(task);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, CantGetRightLength)
{
    HttpTask task;
    HttpTaskUnitTest::setUri(task, "http://www.boost.org/doc/libs/1_39_0/more/getting_started/unix-variants.html");
    HttpTaskUnitTest::setState(task, HttpTask::HT_PREPARE);

    filename = "./cant_get_right_length.download";
    HttpSession session(task);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, PartDownload)
{
    HttpTask task;
    HttpTaskUnitTest::setUri(task, "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html");
    HttpTaskUnitTest::setState(task, HttpTask::HT_PREPARE);

    filename = "./part_download.download";
    HttpSession session(task, 0, 100);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, PartDownloadFromMiddle)
{
    filename = "./part_download_from_middle.download";

    {
        HttpTask task;
        HttpTaskUnitTest::setUri(task, "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html");
        HttpTaskUnitTest::setState(task, HttpTask::HT_PREPARE);

        HttpSession session(task, 0, 50);
        ses = &session;

        CURLcode ret = curl_easy_perform(session.handle());
        if (ret != CURLE_OK)
            printf("meet error: %s\n", curl_easy_strerror(ret));

        ses = NULL;
    }

    {
        HttpTask task;
        HttpTaskUnitTest::setUri(task, "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html");
        HttpTaskUnitTest::setState(task, HttpTask::HT_PREPARE);

        HttpSession session(task, 50, 50);
        ses = &session;

        CURLcode ret = curl_easy_perform(session.handle());
        if (ret != CURLE_OK)
            printf("meet error: %s\n", curl_easy_strerror(ret));

        ses = NULL;
    }
}
