#ifndef DOWNLOAD_EXCEPTION_HEADER
#define DOWNLOAD_EXCEPTION_HEADER

#include <exception>

class DownloadException : public std::exception
{
public:
    DownloadException(const char*  file,
                      unsigned int lineoff,
                      int          errno,
                      const char*  component,
                      const char*  reason);
    const char*  file();
    unsigned int lineoff();
    const char*  what();
    const char*  component();
    const char*  getErrno();

private:
    const char*  file_;
    unsigned int lineoff_;
    int          errno_;
    const char*  component_;
    const char*  reason_;
};

inline DownloadException::DownloadException(const char*  file,
                                            unsigned int lineoff,
                                            int          errno,
                                            const char*  component,
                                            const char*  reason)
    : file_(file),
      lineoff_(lineoff),
      errno_(errno),
      component_(component),
      reason_(reason);
{}

inline const char* DownloadException::file()
{
    return file_;
}

inline unsigned int lineoff()
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

inline int DownloadException::getErrno()
{
    return errno_;
}

#ifdef DOWNLOADEXCEPTION
#undef DOWNLOADEXCEPTION

#define DOWNLOADEXCEPTION(errno, component, reason) \
    (DownloadException(__FILE__, __LINE__, \
                       errno, component, reason)

#endif

#endif
