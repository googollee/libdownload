#ifndef FILE_POSIX_CLASS_HEADER
#define FILE_POSIX_CLASS_HEADER

#include "File.h"

#include <utility/Utility.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace filesystem
{

class FilePosixApi : private Noncopiable
{
public:
    bool open(const char *name, int flag);
    bool isOpen();
    bool close();
    bool resize(size_t len);

    bool read(void *buffer, size_t max, size_t *ret);
    bool write(const void *buffer, size_t len, size_t *ret);

    bool seek(size_t pos, SeekFlag flag);
    bool tell(size_t *ret);

    static int getLastError();
    static const char* strError(int error);

private:
    int fd;

    static int convOpenFlagToNative(int flag);
};

inline int FilePosixApi::convOpenFlagToNative(int flag)
{
    int ret = 0;
    if ( (flag & Read) != 0 && (flag & Write) == 0 )
        ret |= O_RDONLY;
    if ( (flag & Read) == 0 && (flag & Write) != 0 )
        ret |= O_WRONLY;
    if ( (flag & Read) != 0 && (flag & Write) != 0 )
        ret |= O_RDWR;
    if ( (flag & Create) != 0 )
        ret |= O_CREAT;
    if ( (flag & Truncate) != 0 )
        ret |= O_TRUNC;

    return ret;
}

inline int FilePosixApi::getLastError()
{
    return errno;
}

inline const char* FilePosixApi::strError(int error)
{
    return ::strerror(error);
}

inline bool FilePosixApi::isOpen()
{
    return (fd != -1);
}

inline bool FilePosixApi::open(const char *name, int flag)
{
    fd = ::open(name, convOpenFlagToNative(flag) | O_BINARY, 0666);

    return (fd != -1);
}

inline bool FilePosixApi::close()
{
    return (::close(fd) != -1);
}

inline bool FilePosixApi::resize(size_t len)
{
    return (::ftruncate(fd, len) != -1);
}

inline bool FilePosixApi::read(void *buffer, size_t max, size_t *ret)
{
    ssize_t readSize = ::read(fd, buffer, max);
    if (readSize == -1)
    {
        return false;
    }
    *ret = size_t(readSize);
    return true;
}

inline bool FilePosixApi::write(const void *buffer, size_t len, size_t *ret)
{
    ssize_t writeSize = ::write(fd, buffer, len);
    if (writeSize == -1)
    {
        return false;
    }
    *ret = size_t(writeSize);
    return true;
}

inline bool FilePosixApi::seek(size_t pos, SeekFlag flag)
{
    return (::lseek(fd, pos, flag) != -1);
}

inline bool FilePosixApi::tell(size_t *ret)
{
    off_t pos = ::lseek(fd, 0, SEEK_CUR);
    if (pos == -1)
    {
        return false;
    }
    *ret = size_t(pos);
    return true;
}

}

#endif
