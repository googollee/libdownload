#ifndef DOWNLOAD_EXCEPTION_CLASS_HEAD
#define DOWNLOAD_EXCEPTION_CLASS_HEAD

#include <exception>

namespace Utility
{

class DownloadException : public std::exception
{
public:
    DownloadException(const char* file,
                      int         lineoff,
                      int         errorno,
                      const char* component,
                      const char* reason);
    const char* file();
    int         lineoff();
    const char* what();
    const char* component();
    int         error();

private:
    const char* file_;
    int         lineoff_;
    int         errno_;
    const char* component_;
    const char* reason_;
};

inline DownloadException::DownloadException(const char* file,
                                            int         lineoff,
                                            int         errorno,
                                            const char* component,
                                            const char* reason)
    : file_(file),
      lineoff_(lineoff),
      errno_(errorno),
      component_(component),
      reason_(reason)
{}

inline const char* DownloadException::file()
{
    return file_;
}

inline int DownloadException::lineoff()
{
    return lineoff_;
}

inline const char* DownloadException::what()
{
    return reason_;
}

inline const char* DownloadException::component()
{
    return component_;
}

inline int DownloadException::error()
{
    return errno_;
}

}

#define DOWNLOADEXCEPTION(errorno, component, reason)             \
    (throw Utility::DownloadException(__FILE__, __LINE__,         \
                                      errorno, component, reason) \
     )

#endif
