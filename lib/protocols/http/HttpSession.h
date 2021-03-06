#ifndef HTTP_SESSION_HEAD
#define HTTP_SESSION_HEAD

#include <curl/curl.h>

class HttpTask;

class HttpSession
{
public:
    HttpSession(HttpTask& task, size_t pos = 0, long length = UNKNOWN_LEN);
    ~HttpSession();

//     void reset(size_t pos, int length);
    bool checkFinish();
    long getResponseCode();

    HttpTask& task() { return task_; }
    CURL* handle()   { return handle_; }
    size_t pos()     { return pos_; }
    long length()    { return length_; }

    void setLength(long length) { length_ = length; }

private:
    bool initCurlHandle();

    static size_t writeCallback(void *buffer, size_t size, size_t nmemb, HttpSession* ses);

    static const long UNKNOWN_LEN = -1;

    HttpTask& task_; // a reference.
    CURL* handle_;  // a reference.
    size_t pos_;
    long length_;  // -1 mean unknow length.
};

#endif
