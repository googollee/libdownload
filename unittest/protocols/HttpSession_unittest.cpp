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
const char* HttpTask::uri() { return testUri.c_str(); }
const char* HttpTask::outputDir() { return NULL; }
const char* HttpTask::outputName() { return NULL; }
const char* HttpTask::options() { return NULL; }
const char* HttpTask::mimeType() { return NULL; }
const char* HttpTask::comment() { return NULL; }
const char* HttpTask::notice() { return NULL; }
size_t      HttpTask::totalSize() { return 0; }
size_t      HttpTask::downloadSize() { return 0; }
size_t      HttpTask::uploadSize() { return 0; }
int         HttpTask::totalSource() { return 0; }
int         HttpTask::validSource() { return 0; }
std::vector<bool> HttpTask::validBitmap() { return std::vector<bool>(); }
std::vector<bool> HttpTask::downloadBitmap() { return std::vector<bool>(); }
ProtocolBase* HttpTask::protocol() { return NULL; }
void HttpTask::fdSet(fd_set* /*read*/, fd_set* /*write*/, fd_set* /*exc*/, int* /*max*/) {}
size_t HttpTask::performDownload() { return 0; }
size_t HttpTask::performUpload() { return 0; }
int HttpTask::error() { return 0; };
const char* HttpTask::strerror(int error) { error = error; return NULL; }
TaskState HttpTask::state() { return TASK_WAIT; }

HttpTask::InternalState s;

HttpTask::InternalState HttpTask::internalState()
{
    return s;
}

void HttpTask::setInternalState(InternalState state)
{
    s = state;
}

void HttpTask::setError(Error /*error*/)
{}

std::string filename;

void HttpTask::initTask()
{
    file_.open(filename.c_str(), File::OF_Write | File::OF_Create);
    s = HT_DOWNLOAD;

    CURL *ehandle = ses->handle();
    double length;
    curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

    if (length > 0)
        ses->setLength(long(length));
    else
        s = HT_DOWNLOAD_WITHOUT_LENGTH;
}

void HttpTask::sessionFinish(HttpSession* ses)
{
//     CURLcode rete = curl_easy_pause(ses->handle(), CURLPAUSE_ALL);
//     CHECK_CURLE(rete);

    s = HT_FINISH;

    file_ .close();
}

void HttpTask::seekFile(size_t pos)
{
    if (!file_.seek(pos, File::SF_FromBegin))
    {
        s = HT_ERROR;
        printf("seek fail\n");
    }
}

ssize_t HttpTask::writeFile(void* buffer, size_t size)
{
    ssize_t ret = file_.write(buffer, size);
    if (ret == -1)
    {
        s = HT_ERROR;
    }

    return ret;
}

TEST(HttpSessionTest, Normal)
{
    testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
    s = HttpTask::HT_PREPARE;
    filename = "./normal.download";

    HttpTask task;
    HttpSession session(task);
    ses = &session;

    CURLcode ret = curl_easy_perform(session.handle());
    if (ret != CURLE_OK)
        printf("meet error: %s\n", curl_easy_strerror(ret));

    ses = NULL;
}

TEST(HttpSessionTest, CantGetRightLength)
{
    testUri = "http://www.boost.org/doc/libs/1_39_0/more/getting_started/unix-variants.html";
    s = HttpTask::HT_PREPARE;
    filename = "./cant_get_right_length.download";

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
    s = HttpTask::HT_PREPARE;
    filename = "./part_download.download";

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
    filename = "./part_download_from_middle.download";

    {
        testUri = "http://curl.haxx.se/libcurl/c/curl_easy_getinfo.html";
        s = HttpTask::HT_PREPARE;

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
        s = HttpTask::HT_PREPARE;

        HttpTask task;
        HttpSession session(task, 50, 50);
        ses = &session;

        CURLcode ret = curl_easy_perform(session.handle());
        if (ret != CURLE_OK)
            printf("meet error: %s\n", curl_easy_strerror(ret));

        ses = NULL;
    }
}
