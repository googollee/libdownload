#include "HttpTask.h"

#include <boost/format.hpp>

HttpTask::HttpTask()
{}

HttpTask::~HttpTask()
{}

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

void HttpTask::fdSet(fd_set* read, fd_set* write, fd_set* exc, int* max)
size_t HttpTask::performDownload()
size_t HttpTask::performUpload()

const char* HttpTask::strerror(int error)

void HttpTask::setInternalState(InternalState state)
void HttpTask::setError(Error error, const char* errstr = NULL)

void HttpTask::initTask()
void HttpTask::sessionFinish(HttpSession* ses)

void HttpTask::seekFile(size_t pos)
ssize_t HttpTask::writeFile(void *buffer, size_t size)
