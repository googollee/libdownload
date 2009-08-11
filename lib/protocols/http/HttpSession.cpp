#include "HttpSession.h"
#include "HttpTask.h"
#include "utility/Utility.h"

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

    if (length == 0)
        throw DOWNLOADEXCEPTION(1, "HTTP", "length can't be 0.");

    if (handle_ == NULL)
    {
        task_.setError(HttpTask::CURL_BAD_ALLOC);
        return;
    }

    initCurlHandle();
}

HttpSession::~HttpSession()
{
    curl_easy_cleanup(handle_);
}

// void HttpSession::reset(size_t pos, int length)
// {
//     if (length == 0)
//         throw DOWNLOADEXCEPTION(1, "HTTP", "length can't be 0.");

//     pos_ = pos;
//     length_ = length;

//     curl_easy_reset(handle_);

//     initCurlHandle();
// }

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
    if (rete != CURLE_OK)
        throw DOWNLOADEXCEPTION(1, "CURL", curl_easy_strerror(rete));

    return ret;
}

bool HttpSession::initCurlHandle()
{
#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
        {                                                               \
            task_.setError(HttpTask::OTHER, curl_easy_strerror(rete));  \
            return false;                                               \
        }                                                               \
    }

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task_.uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, &HttpSession::writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_PRIVATE, this);
    CHECK_CURLE(rete);

    if (task_.configure().referer.length() != 0)
    {
        rete = curl_easy_setopt(handle_, CURLOPT_REFERER, task_.configure().referer.c_str());
        CHECK_CURLE(rete);
    }

    if (task_.configure().userAgent.length() != 0)
    {
        rete = curl_easy_setopt(handle_, CURLOPT_USERAGENT, task_.configure().userAgent.c_str());
        CHECK_CURLE(rete);
    }

    if (task_.configure().connectingTimeout > 0)
    {
        rete = curl_easy_setopt(handle_, CURLOPT_CONNECTTIMEOUT, task_.configure().connectingTimeout);
        CHECK_CURLE(rete);
    }

    if (length_ != UNKNOWN_LEN)
    {
        char range[128] = {0};
        sprintf(range, "%lu-%lu", pos_, pos_ + length_ - 1);
        rete = curl_easy_setopt(handle_, CURLOPT_RANGE, range);
        CHECK_CURLE(rete);
    }

    return true;
#undef CHECK_CURLE
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
