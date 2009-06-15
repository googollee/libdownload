#ifndef FILE_POSIX_CLASS_HEADER
#define FILE_POSIX_CLASS_HEADER

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

#define OF_READ     O_RDONLY
#define OF_WRITE    O_WRONLY
#define OF_RDWR     O_RDWR
#define OF_CREATE   O_CREAT
#define OF_TRUNCATE O_TRUNC

#define SF_BEGIN   SEEK_SET
#define SF_CURRENT SEEK_CUR
#define SF_END     SEEK_END


namespace Utility
{

class FileApi
{
public:
    typedef int HANDLE;

    static int getLastError();
    static const char* strError(int error);

    static bool exist(const char *name);
    static bool remove(const char *name);
    static bool resize(const char *name, size_t len);

    static void initHandle(HANDLE *handle);

    static bool open(const char *name, int flag, HANDLE *handle);
    static bool isOpen(HANDLE &handle);
    static bool isEof(HANDLE &handle);
    static bool close(HANDLE &handle);

    static bool read(HANDLE &handle, void *buffer, size_t count, size_t *readSize);
    static bool write(HANDLE &handle, const void *buffer, size_t count, size_t *writeSize);

    static bool seek(HANDLE &handle, size_t pos, int flag);
    static bool tell(HANDLE &handle, size_t *ret);
};

inline int FileApi::getLastError()
{
    return errno;
}

inline const char* FileApi::strError(int error)
{
    return ::strerror(error);
}

inline bool FileApi::exist(const char *name )
{
    return (::access(name, F_OK) == 0);
}

inline bool FileApi::remove(const char *name )
{
    return  (::remove(name) == 0);
}

inline bool FileApi::resize(const char *name, size_t len)
{
    HANDLE handle;
    if (!FileApi::open(name, O_RDWR, &handle))
        return false;

    int ret = ::ftruncate(handle, len);

    FileApi::close(handle);

    return (ret == 0);
}

inline void FileApi::initHandle(HANDLE *handle)
{
    *handle = -1;
}

inline bool FileApi::open(const char *name, int flag, HANDLE *handle)
{
#ifdef O_BINARY
    *handle = ::open(name, flag | O_BINARY , 0666);
#else
    *handle = ::open(name, flag, 0666);
#endif

    return (*handle != -1);
}

inline bool FileApi::isEof(HANDLE &handle)
{
    off_t pos = ::lseek(handle, 0, SEEK_CUR);
    off_t end = ::lseek(handle, 0, SEEK_END);

    ::lseek(handle, pos, SEEK_SET);

    return pos == end;
}

inline bool FileApi::isOpen(HANDLE &handle)
{
    return (handle != -1);
}

inline bool FileApi::close(HANDLE &handle)
{
    if (::close(handle) == -1)
    {
        return false;
    }

    return true;
}

inline bool FileApi::read(HANDLE &handle, void *buffer, size_t count, size_t *readSize)
{
    ssize_t got = 0;

    got = ::read(handle, buffer, count);

    if (got == -1)
        return false;

    if (readSize != NULL)
        *readSize = got;

    return true;
}

inline bool FileApi::write(HANDLE &handle, const void *buffer, size_t count, size_t *writeSize)
{
    ssize_t got = 0, need = count;
    const char *buf = static_cast<const char *>(buffer);

    while ((got = ::write(handle, buf, need)) > 0 && (need -= got) > 0)
        buf += got;

    if (writeSize != NULL)
        *writeSize = count - need;

    return got != -1;
}

inline bool FileApi::seek(HANDLE &handle, size_t pos, int flag)
{
    return (::lseek(handle, pos, flag) != -1);
}

inline bool FileApi::tell(HANDLE &handle, size_t *ret)
{
    off_t pos = ::lseek(handle, 0, SEEK_CUR);
    if (pos == -1)
    {
        return false;
    }
    *ret = size_t(pos);
    return true;
}

}

#endif
