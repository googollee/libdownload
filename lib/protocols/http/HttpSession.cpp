#include "HttpSession.h"

#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
            throw DOWNLOADEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

HttpSession::HttpSession(HttpTask<File>& task, size_t pos, long length)
    : task_(task),
      handle_(curl_easy_init()),
      pos_(pos),
      length_(length)
{
    LOG(0, "make task %p session from %lu, len %lu\n", &task_, pos, length);
    {
        char logBuffer[64] = {0};
        snprintf(logBuffer, 63, "make new session from %lu, len %lu", pos, length);
        task_.log(logBuffer);
    }

    if (handle_ == NULL)
        throw DOWNLOADEXCEPTION(HttpTask<File>::CURL_BAD_ALLOC, "CURL", task_.strerror(HttpTask<File>::CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task_.getUri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &HttpSession<Utility::File>::writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
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

HttpSession::~HttpSession()
{
    curl_easy_cleanup(handle_);
}

void HttpSession::reset(size_t pos, int length)
{
    pos_ = pos;
    length_ = length;

    curl_easy_reset(handle_);

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task.uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
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
    LOG(0, "check task %p session %p from %lu, len %l\n",
        &task_, this, pos_, length_);

    if (length_ > 0 || length_ == UNKNOWN_LEN) // unknown length will disconnect directly, no need check here.
    {
        return false;
    }

    CURLcode rete = curl_easy_pause(handle_, CURLPAUSE_ALL);
    CHECK_CURLE(rete);

    return true;
}

long HttpSession::getResponseCode()
{
    long ret = 0;
    CURLcode rete = curl_easy_getinfo(handle_, CURLINFO_RESPONSE_CODE, &ret);
    CHECK_CURLE(rete);

    return ret;
}

size_t HttpSession::writeCallback(void *buffer, size_t size, size_t nmemb, HttpSession *ses)
{
    if (ses->task_.internalState() == HttpTask<File>::HT_PREPARE)
    {
        ses->task_.initTask();
    }

    size_t shouldWrite = size;
    if (ses->length_ > 0)
    {
        // got the file size from server.
        if (size > size_t(length_))
            shouldWrite = length_;

        ses->task_.seekFile(pos_);
    }

    const size_t writed = ses->task_.writeFile(buffer, shouldWrite);

    if (ses->length_ > 0)
    {
        ses->length_ -= writed;
        ses->pos_ += writed;
    }

    if (ses->checkFinish())
        ses->task_.sessionFinish(ses);

    return writed;
}
