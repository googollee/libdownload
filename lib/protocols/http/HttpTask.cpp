#include "HttpTask.h"

#include <boost/format.hpp>

#include "HttpSession.h"

HttpTask::HttpTask()
    : totalSize_(0),
      downloadSize_(0),
      totalSource_(0),
      validSource_(0),
      state_(TASK_WAIT),
      protocol_(NULL),
      err_(OTHER),
      internalState_(HT_INVALID),
      handle_(curl_multi_init()),
      writeLength_(0),
      lastRunningHandle_(0)
{
    if (handle_ == NULL)
    {
        setError(CURLM_BAD_ALLOC);
        LOG(0, "create multi handle faile.\n");
        return;
    }
}

HttpTask::~HttpTask()
{
    for (int i=0, n=sessions_.size(); i<n; ++i)
    {
        sessionFinish(sessions_[i]);
    }

    CURLMcode retm =  curl_multi_cleanup(handle_);
    if (retm != CURLM_OK)
    {
        LOG(0, "clean up multi handle fail: %s.", curl_multi_strerror(retm));
    }
}

const char* HttpTask::options()
{
    static std::string buffer;

    buffer = str(
        boost::format("<SessionNumber>%d</SessionNumber>"
                      "<MinSessionBlocks>%d</MinSessionBlocks>"
                      "<BytesPerBlock>%d</BytesPerBlock>"
                      "<Referer>%s</Referer>"
                      "<UserAgent>%s</UserAgent>"
                      "<RetryCount>%d</RetryCount>"
                      "<ConnectingTimeOut>%ld</ConnectingTimeOut>"
            )
        % config_.sessionNumber
        % config_.minSessionBlocks
        % config_.bytesPerBlock
        % config_.referer
        % config_.userAgent
        % config_.retryCount
        % config_.connectingTimeout);

    return buffer.c_str(); // buffer is static varable, so it should be OK.
}

bool HttpTask::start()
{
    HttpSession* ses = new HttpSession(*this);
    if (ses == NULL)
    {
        LOG(0, "create easy handle faile.\n");
        return false;
    }

    sessions_.push_back(ses);
    CURLMcode retm = curl_multi_add_handle(handle_, ses->handle());
    if (retm != CURLM_OK)
    {
        setError(OTHER, curl_multi_strerror(retm));
        LOG(0, "add easy handle to multi handle fail: %s.\n", curl_multi_strerror(retm));
        return false;
    }

    setInternalState(HT_PREPARE);

    return true;
}

bool HttpTask::stop()
{
    return false;
}

bool HttpTask::fdSet(fd_set* read, fd_set* write, fd_set* exc, int* max)
{
    CURLMcode ret = curl_multi_fdset(handle_, read, write, exc, max);
    if (ret != CURLM_OK)
    {
        setError(OTHER, curl_multi_strerror(ret));
        return false;
    }

    return true;
}

size_t HttpTask::performDownload()
{
    writeLength_ = 0;
    CURLMcode ret;
    int running = 0;
    while ((ret = curl_multi_perform(handle_, &running)) == CURLM_CALL_MULTI_PERFORM)
    {}

    if (running < lastRunningHandle_)
    {
        hasSessionFinish();
    }
    lastRunningHandle_ = running;

    clearSessions();

    return writeLength_;
}

size_t HttpTask::performUpload()
{
    clearSessions();

    return 0;
}

const char* HttpTask::strerror(int error)
{
    return NULL;
}

void HttpTask::setInternalState(InternalState state)
{
    internalState_ = state;
    switch (internalState_)
    {
    case HT_INVALID:
    case HT_ERROR:
        state_ = TASK_ERROR;
        break;
    case HT_PREPARE:
    case HT_DOWNLOAD:
    case HT_DOWNLOAD_WITHOUT_LENGTH:
        state_ = TASK_DOWNLOAD;
        break;
    case HT_FINISH:
        state_ = TASK_FINISH;
        break;
    default:
        state_ = TASK_ERROR;
        break;
    }
}

void HttpTask::setError(Error error, const char* errstr)
{
    setInternalState(HT_ERROR);
    err_ = error;
    errstr_ = errstr;
}

static std::string guessFileName(const std::string uri)
{
    size_t p = uri.find_last_of("*|\\:\"<>?/") + 1;
    return uri.substr(p);
}

void HttpTask::initTask()
{
    std::string filename = outputDir_;
    if (outputName_.length() == 0)
    {
        outputName_ = guessFileName(uri_);
    }
    filename += outputName_;

    file_.open(filename.c_str(), Utility::FileManager::OF_Write | Utility::FileManager::OF_Create);

    HttpSession* ses = sessions_[0];

    CURL* ehandle = ses->handle();
    double length;
    CURLcode ret = curl_easy_getinfo(ehandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
    if (ret != CURLE_OK)
    {
        setError(OTHER, curl_easy_strerror(ret));
        return;
    }

    char* contentType = NULL;
    ret = curl_easy_getinfo(ehandle, CURLINFO_CONTENT_TYPE, &contentType);
    if (ret != CURLE_OK)
    {
        setError(OTHER, curl_easy_strerror(ret));
        return;
    }
    if (contentType != NULL)
    {
        char* split = strchr(contentType, ';');
        if (split == NULL)
            mimeType_ = contentType;
        else
            mimeType_ = std::string(contentType, split - contentType);
    }

    validSource_ = totalSource_ = 1;

    if (length > 0)
    {
        ses->setLength(long(length));
        setInternalState(HT_DOWNLOAD);
        totalSize_ = size_t(length);
        downloadBitmap_ = validBitmap_ = BitMap(totalSize_, config_.bytesPerBlock);
        validBitmap_.setAll(true);
        downloadBitmap_.setAll(false);

        separateSession();
    }
    else
    {
        setInternalState(HT_DOWNLOAD_WITHOUT_LENGTH);
    }
}

void HttpTask::separateSession()
{
}

void HttpTask::sessionFinish(HttpSession* ses)
{
    CURLcode rete = curl_easy_pause(ses->handle(), CURLPAUSE_ALL);
    if (rete != CURLE_OK)
    {
        setError(HttpTask::OTHER, curl_easy_strerror(rete));
        LOG(0, "can't paush easy handle: %s", curl_easy_strerror(rete));
    }

    Sessions::iterator it = std::find(sessions_.begin(), sessions_.end(), ses);
    if (it == sessions_.end())
    {
        setError(HttpTask::OTHER, "can't find session.");
        LOG(0, "can't find session: %p.", ses);
    }
    sessions_.erase(it);

    finishedSessions_.push_back(ses);
}

void HttpTask::clearSessions()
{
    for (int i=0, n=finishedSessions_.size(); i<n; ++i)
    {
        HttpSession* ses = finishedSessions_[i];

        CURLMcode retm = curl_multi_remove_handle(handle_, ses->handle());
        if (retm != CURLM_OK)
        {
            setError(HttpTask::OTHER, curl_multi_strerror(retm));
            LOG(0, "can't remove easy handle: %s", curl_multi_strerror(retm));
        }

        delete ses;
    }

    finishedSessions_.clear();

    if (checkFinish())
    {
        setInternalState(HT_FINISH);
        file_.close();
    }
}

bool HttpTask::writeFile(size_t pos, void *buffer, size_t size)
{
    if (internalState_ == HT_DOWNLOAD)
    {
        if (!file_.seek(pos, Utility::FileManager::SF_FromBegin))
        {
            setError(FAIL_FILE_IO);
            return false;
        }
    }

    ssize_t ret = file_.write(buffer, size);
    if (ret == -1)
    {
        setError(FAIL_FILE_IO);
        return false;
    }

    downloadSize_ += size;

    if (internalState_ == HT_DOWNLOAD)
    {
        printf("write: %lu-%lu\n", pos, pos+size);
        downloadBitmap_.setRangeByLength(pos, pos + size, true);
    }

    return true;
}

void HttpTask::hasSessionFinish()
{
    // check task status.
    CURLMsg *msg = NULL;
    int msgsInQueue;
    while ( (msg = curl_multi_info_read(handle_, &msgsInQueue)) != NULL)
    {
        HttpSession *ses = NULL;
        CURLcode rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &ses);
        if (rete != CURLE_OK)
        {
            setError(HttpTask::OTHER, curl_easy_strerror(rete));
        }

        long respCode = 0;
        rete = curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &respCode);
        if (rete != CURLE_OK)
        {
            setError(HttpTask::OTHER, curl_easy_strerror(rete));
        }

        int topRespCode = respCode / 100;
        LOG(0, "top response code = %d\n", topRespCode);

        switch (msg->msg)
        {
        case CURLMSG_DONE:
            switch (topRespCode)
            {
            case 2: // succeed download
                sessionFinish(ses);
                break;
            default:
                break;
            }
            break;
            // TODO: handle other return code.
        default:
            break;
        }
    }
}

bool HttpTask::checkFinish()
{
    return sessions_.size() == 0;
}
