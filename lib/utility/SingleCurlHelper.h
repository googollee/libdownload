#ifndef SINGLE_CURL_HELPER_HEADER
#define SINGLE_CURL_HELPER_HEADER

#include <utility/Utility.h>
#include <utility/DownloadException.h>

#include <curl/curl.h>

class SingleCurlHelper : public Noncopiable
{
public:
    static void init()
        {
            static SingleCurlHelper helper;
        }

private:
    SingleCurlHelper()
        {
            CURLcode rete = curl_global_init(CURL_GLOBAL_ALL);
            if (rete != CURLE_OK)
                throw DOWNLOADEXCEPTION(rete, "CURLE", curl_easy_strerror(rete));
        }

    ~SingleCurlHelper()
        {
            curl_global_cleanup();
        }
};

#define CHECK_CURLE(rete)                                               \
    {                                                                   \
        if (rete != CURLE_OK)                                           \
            throw DOWNLOADEXCEPTION(rete, "CURL", curl_easy_strerror(rete)); \
    }

#endif
