#include "utility/File.h"

#include "protocols/http/HttpTask.h"
#include "protocols/http/HttpSession.inc"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

std::string uri;
HttpSession<File> *ses = NULL;

template<typename File>
HttpTask<File>::HttpTask() {}
template<typename File>
HttpTask<File>::~HttpTask() {}
template<typename File>
const char *HttpTask<File>::getUri() { return uri.c_str(); }
template<typename File>
const char *HttpTask<File>::getOutputDir() { return NULL; }
template<typename File>
const char *HttpTask<File>::getOutputName() { return NULL; }
template<typename File>
const char *HttpTask<File>::getOptions() { return NULL; }
template<typename File>
const char *HttpTask<File>::getMimeType() { return NULL; }
template<typename File>
const char *HttpTask<File>::getComment() { return NULL; }
template<typename File>
size_t      HttpTask<File>::getTotalSize() { return 0; }
template<typename File>
size_t      HttpTask<File>::getDownloadSize() { return 0; }
template<typename File>
size_t      HttpTask<File>::getUploadSize() { return 0; }
template<typename File>
int         HttpTask<File>::getTotalSource() { return 0; }
template<typename File>
int         HttpTask<File>::getValidSource() { return 0; }
template<typename File>
std::vector<bool> HttpTask<File>::getValidBitmap() { return std::vector<bool>(); }
template<typename File>
std::vector<bool> HttpTask<File>::getDownloadBitmap() { return std::vector<bool>(); }
template<typename File>
ProtocolBase *HttpTask<File>::getProtocol() { return NULL; }
template<typename File>
void HttpTask<File>::getFdSet(fd_set *read, fd_set *write, fd_set *exc, int *max) {}
template<typename File>
size_t HttpTask<File>::performDownload() { return 0; }
template<typename File>
size_t HttpTask<File>::performUpload() { return 0; }
template<typename File>
const char *HttpTask<File>::strerror(int error) { error = error; return NULL; }
template<typename File>
TaskState HttpTask<File>::getState() { return TASK_WAIT; }

int s = 0;

template<typename File>
typename HttpTask<File>::InternalState HttpTask<File>::internalState()
{
    return (typename HttpTask<File>::InternalState)s;
}

template<typename File>
void HttpTask<File>::initTask()
{
    file.open("./test.data", File::OF_Write | File::OF_Create);
    s = (int)HT_DOWNLOAD;

    CURL *ehandle = ses->handle();
    double length;
    curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

    ses->setLength(long(length));
}

template<typename File>
void HttpTask<File>::sessionFinish(HttpSession<File> *ses)
{
    file.close();
}

template<typename File>
void HttpTask<File>::seekFile(size_t pos)
{
    file.seek(pos, File::SF_FromBegin);
}

template<typename File>
size_t HttpTask<File>::writeFile(void *buffer, size_t size)
{
    return file.write(buffer, size);
}

TEST(HttpSessionTest, Normal)
{
    HttpTask<File> task;
    HttpSession<File> session(task);
    ses = &session;

    curl_easy_perform(session.handle());

    ses = NULL;
}
