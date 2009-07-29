#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.h"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

std::string uri;
HttpSession *ses = NULL;

HttpTask::HttpTask() {}
HttpTask::~HttpTask() {}
const char *HttpTask::getUri() { return uri.c_str(); }
const char *HttpTask::getOutputDir() { return NULL; }
const char *HttpTask::getOutputName() { return NULL; }
const char *HttpTask::getOptions() { return NULL; }
const char *HttpTask::getMimeType() { return NULL; }
const char *HttpTask::getComment() { return NULL; }
size_t      HttpTask::getTotalSize() { return 0; }
size_t      HttpTask::getDownloadSize() { return 0; }
size_t      HttpTask::getUploadSize() { return 0; }
int         HttpTask::getTotalSource() { return 0; }
int         HttpTask::getValidSource() { return 0; }
std::vector<bool> HttpTask::getValidBitmap() { return std::vector<bool>(); }
std::vector<bool> HttpTask::getDownloadBitmap() { return std::vector<bool>(); }
ProtocolBase *HttpTask::getProtocol() { return NULL; }
void   HttpTask::getFdSet(fd_set *read, fd_set *write, fd_set *exc, int *max) {}
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

    ses->setLength(long(length));
}

void HttpTask::sessionFinish(HttpSession *ses)
{
    file.close();
}

void HttpTask::seekFile(size_t pos)
{
    file.seek(pos, File::SF_FromBegin);
}

size_t HttpTask::writeFile(void *buffer, size_t size)
{
    return file.write(buffer, size);
}

TEST(HttpSessionTest, Normal)
{
    HttpTask task;
    HttpSession session(task);
    ses = &session;

    curl_easy_perform(session.handle());

    ses = NULL;
}
