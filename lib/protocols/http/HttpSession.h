#ifndef HTTP_SESSION_HEAD
#define HTTP_SESSION_HEAD

#include "HttpTask.h"

#include <curl/curl.h>

class HttpSession
{
public:
    HttpSession(HttpTask* task, size_t pos = 0, int length = -1);
    ~HttpSession();

    void reset(size_t pos, int length);
    bool checkWorking();
    long getResponseCode();

    size_t write(void *buffer, size_t size);

    HttpTask* task() { return task_; }
    CURL* handle()   { return handle_; }
    size_t pos()     { return pos_; }
    int length()     { return length_; }

    void setLength(int length) { length_ = length; }

private:
    HttpTask* task_; // a reference.
    CURL* handle_;  // a reference.
    size_t pos_;
    int length_;  // -1 mean unknow length.
};

#endif
