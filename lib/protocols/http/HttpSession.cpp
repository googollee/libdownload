#include "HttpSession.h"

HttpSession::HttpSession(HttpTask* task, size_t pos, int length)
    : task_(task),
      handle_(curl_easy_init()),
      pos_(pos),
      length(length)
{
    LOG(0, "make task %p session from %lu, len %lu\n", task_, begin, len);
    {
        char logBuffer[64] = {0};
        snprintf(logBuffer, 63, "make new session from %lu, len %lu", begin, len);
        task_->taskLog(logBuffer);
    }

    if (handle_ == NULL)
        throw DOWNLOADEXCEPTION(CURL_BAD_ALLOC, "CURL", HttpTask::strerror(CURL_BAD_ALLOC));

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task->uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    if (length != -1)
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

    CURLcode rete = curl_easy_setopt(handle_, CURLOPT_URL, task->uri());
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(handle_, CURLOPT_WRITEDATA, this);
    CHECK_CURLE(rete);

    rete = curl_easy_setopt(ses->handle, CURLOPT_PRIVATE, ses.get());
    CHECK_CURLE(rete);

    if (length != -1)
    {
        char range[128] = {0};
        sprintf(range, "%lu-%lu", pos_, length_);
        rete = curl_easy_setopt(handle_, CURLOPT_RANGE, range);
        CHECK_CURLE(rete);
    }
}

bool HttpSession::checkWorking()
{
    LOG(0, "check task %p session %p from %lu, len %d\n",
        task_, this, pos_, length_);

    if (length_ > 0) // length == -1 will disconnect directly, no need check here.
    {
        return true;
    }

    size_t begin;
    size_t len;
    if (task->getNonDownload(&begin, &len))
    {
        this->reset(begin, len);
        return true;
    }

    CURLcode rete = curl_easy_pause(ses->handle, CURLPAUSE_ALL);
    CHECK_CURLE(rete);
    task_->sessionFinish(this);

    return false;
}

long HttpSession::getResponseCode()
{
    long ret = 0;
    rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &ret);
    CHECK_CURLE(rete);

    return ret;
}

size_t HttpSession::write(void *buffer, size_t size)
{
    if (task_->state() == HT_PREPARE)
    {
        task_->initTask();
    }

    size_t shouldWrite = size;
    if (length_ > 0)
    {
        // got the file size from server.
        if (size > size_t(length_))
            shouldWrite = length_;

        task_->seekFile(pos_);
    }

    const size_t writed = task_->writeFile(buffer, shouldWrite);

    if (length_ > 0)
    {
        length_ -= writed;
        pos_ += writed;
    }

    checkFinish();

    return writed;
}
