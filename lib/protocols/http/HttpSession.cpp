#include "HttpSession.h"
#include "HttpTask.h"
#include "utility/Utility.h"

#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
            throw DOWNLOADEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

HttpSession::HttpSession(HttpTask& task, size_t pos, long length)
    : task_(task),
      handle_(curl_easy_init()),
      pos_(pos),
      length_(length)
{
    LOG(0, "make task %p session from %lu, len %ld\n", &task_, pos, length);
    {
        char logBuffer[64] = {0};
        snprintf(logBuffer, 63, "make new session from %lu, len %lu", pos, length);
        task_.log(logBuffer);
    }

    if (handle_ == NULL)
        throw DOWNLOADEXCEPTION(HttpTask::CURL_BAD_ALLOC, "CURL", task_.strerror(HttpTask::CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task_.uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &HttpSession::writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_PRIVATE, this);
    CHECK_CURLE(rete);

    if (length == 0)
        throw DOWNLOADEXCEPTION(1, "HTTP", "length can't be 0.");

    if (length != UNKNOWN_LEN)
    {
        char range[128] = {0};
        sprintf(range, "%lu-%lu", pos_, pos_ + length_ - 1);
        printf("range: %s\n", range);
        rete = curl_easy_setopt(handle_, CURLOPT_RANGE, range);
        CHECK_CURLE(rete);
    }
}

HttpSession::~HttpSession()
{
    curl_easy_cleanup(handle_);
}

void HttpSession::reset(size_t pos, int length)
{
    pos_ = pos;
    length_ = length;

    curl_easy_reset(handle_);

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task_.uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &HttpSession::writeCallback);
    CHECK_CURLE(rete);

    void* data = this;
    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, data);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_PRIVATE, this);
    CHECK_CURLE(rete);

    if (length != UNKNOWN_LEN)
    {
        char range[128] = {0};
        sprintf(range, "%lu-%lu", pos_, length_);
        rete = curl_easy_setopt(handle_, CURLOPT_RANGE, range);
        CHECK_CURLE(rete);
    }
}

bool HttpSession::checkFinish()
{
    LOG(0, "check task %p session %p from %lu, len %ld\n",
        &task_, this, pos_, length_);

    if (length_ > 0 || length_ == UNKNOWN_LEN)
    {
        return false;
    }

    return true;
}

long HttpSession::getResponseCode()
{
    long ret = 0;
    CURLcode rete = curl_easy_getinfo(handle_, CURLINFO_RESPONSE_CODE, &ret);
    CHECK_CURLE(rete);

    return ret;
}

size_t HttpSession::writeCallback(void *buffer, size_t size, size_t nmemb, HttpSession* ses)
{
    if (ses->task_.internalState() == HttpTask::HT_PREPARE)
    {
        ses->task_.initTask();
    }

    ssize_t shouldWrite = size * nmemb;
    if (ses->length_ > 0)
    {
        // got the file size from server.
        if (shouldWrite > ses->length_)
            shouldWrite = ses->length_;

        ses->task_.seekFile(ses->pos_);
    }

    const ssize_t writed = ses->task_.writeFile(buffer, shouldWrite);
    if (writed == -1)
    {
        //HttpTask should change state to error in writeFile if fail.
        LOG(0, "write fail\n");
        return 0;
    }

    if (ses->length_ > 0)
    {
        ses->length_ -= writed;
        ses->pos_ += writed;
    }

    if (ses->checkFinish())
        ses->task_.sessionFinish(ses);

    return (writed == shouldWrite) ? (size * nmemb) : writed;
}
