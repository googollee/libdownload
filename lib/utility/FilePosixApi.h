#ifndef FILE_POSIX_CLASS_HEADER
#define FILE_POSIX_CLASS_HEADER

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

namespace Utility
{

class File
{
private:
    typedef int HANDLE;

public:
    enum OpenFlag
    {
        OF_Read     = O_RDONLY,
        OF_Write    = O_WRONLY,
        OF_RW       = O_RDWR,
        OF_Create   = O_CREAT,
        OF_Truncate = O_TRUNC,
    };

    enum SeekFlag
    {
        SF_FromBegin   = SEEK_SET,
        SF_FromCurrent = SEEK_CUR,
        SF_FromEnd     = SEEK_END,
    };

    static int getLastError();
    static const char* strError(int error);

    static bool exist(const char *name);
    static bool remove(const char *name);
    static bool resize(const char *name, size_t len);

    File();
    ~File();

    bool open(const char *name, int flag);
    bool isOpen();
    bool isEof();
    bool close();

    ssize_t read(void *buffer, size_t count);
    ssize_t write(const void *buffer, size_t count);

    bool seek(size_t pos, int flag);
    ssize_t tell();

private:
    HANDLE handle_;
};

inline int File::getLastError()
{
    return errno;
}

inline const char* File::strError(int error)
{
    return ::strerror(error);
}

inline bool File::exist(const char *name )
{
    return (::access(name, F_OK) == 0);
}

inline bool File::remove(const char *name )
{
    return  (::remove(name) == 0);
}

inline bool File::resize(const char *name, size_t len)
{
    File f;

    if (!f.open(name, OF_RW))
        return false;

    int ret = ::ftruncate(f.handle_, len);

    f.close();

    return (ret == 0);
}

inline File::File()
    : handle_(-1)
{}

inline File::~File()
{
    if (handle_ != -1)
        close();
}

inline bool File::open(const char *name, int flag)
{
#ifdef O_BINARY
    handle_ = ::open(name, flag | O_BINARY , 0666);
#else
    handle_ = ::open(name, flag, 0666);
#endif

    return (handle_ != -1);
}

inline bool File::isEof()
{
    off_t pos = ::lseek(handle_, 0, SEEK_CUR);
    off_t end = ::lseek(handle_, 0, SEEK_END);

    ::lseek(handle_, pos, SEEK_SET);

    return pos == end;
}

inline bool File::isOpen()
{
    return (handle_ != -1);
}

inline bool File::close()
{
    if (::close(handle_) == -1)
    {
        return false;
    }
    handle_ = -1;

    return true;
}

inline ssize_t File::read(void *buffer, size_t count)
{
    return ::read(handle_, buffer, count);
}

inline ssize_t File::write(const void *buffer, size_t count)
{
    ssize_t got = 0, need = count;
    const char *buf = static_cast<const char *>(buffer);

    while ((got = ::write(handle_, buf, need)) > 0 && (need -= got) > 0)
        buf += got;

    if (got == -1)
        return -1;

    return count - need;
}

inline bool File::seek(size_t pos, int flag)
{
    return (::lseek(handle_, pos, flag) != -1);
}

inline ssize_t File::tell()
{
    return ::lseek(handle_, 0, SEEK_CUR);
}

}

#endif
