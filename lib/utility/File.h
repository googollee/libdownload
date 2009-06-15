#ifndef DOWNLOAD_FILE_CLASS_HEAD
#define DOWNLOAD_FILE_CLASS_HEAD

#define OF_READ     0x1
#define OF_WRITE    0x2
#define OF_CREATE   0x4
#define OF_TRUNCATE 0x8

#define SF_BEGIN   0
#define SF_CURRENT 1
#define SF_END     2

#include "FilePosixApi.h"
#include "Utility.h"

#include <stdio.h>

namespace Utility
{

class File : private Noncopiable
{
public:
    enum OpenFlag
    {
        OF_Read     = OF_READ,
        OF_Write    = OF_WRITE,
        OF_Create   = OF_CREATE,
        OF_Truncate = OF_TRUNCATE,
    };

    enum SeekFlag
    {
        SF_FromBegin   = SF_BEGIN,
        SF_FromCurrent = SF_CURRENT,
        SF_FromEnd     = SF_END,
    };

    static bool exist(const char *name);
    static void remove(const char *name);
    static void resize(const char *name, size_t len);

    File();
    ~File();

    void open(const char *name, int flag);
    bool isOpen();
    void close();

    size_t read(void *buffer, size_t max);
    size_t write(const void *buffer, size_t len);

    void seek(size_t pos, SeekFlag flag);
    size_t tell();

private:
    FileApi::HANDLE handle_;
};

inline File::File()
{
    FileApi::initHandle(&handle_);
}

inline File::~File()
{
    if (this->isOpen())
    {
        close();
    }
}

inline bool File::exist(const char *name)
{
    return FileApi::exist(name);
}

inline void File::remove(const char *name)
{
    if ( !FileApi::remove(name) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
}

inline void File::resize(const char *name, size_t len)
{
    if ( !FileApi::resize(name, len) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
}

inline void File::open(const char *name, int flag)
{
    if ( !FileApi::open(name, flag, &handle_) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
}

inline bool File::isOpen()
{
    return FileApi::isOpen(handle_);
}

inline void File::close()
{
    if ( !FileApi::close(handle_) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }

    FileApi::initHandle(&handle_);
}

inline size_t File::read(void *buffer, size_t max)
{
    if (!this->isOpen())
    {
        DOWNLOADEXCEPTION(-1, "File", "File doesn't open.");
    }

    size_t ret = 0;
    if ( !FileApi::read(handle_,buffer, max, &ret) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
    return ret;
}

inline size_t File::write(const void *buffer, size_t len)
{
    size_t ret = 0;
    if ( !FileApi::write(handle_, buffer, len, &ret) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
    return ret;
}

inline void File::seek(size_t pos, SeekFlag flag)
{
    if ( !FileApi::seek(handle_, pos, flag) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
}

inline size_t File::tell()
{
    size_t ret;
    if ( !FileApi::tell(handle_, &ret) )
    {
        int err = FileApi::getLastError();
        DOWNLOADEXCEPTION(err, "File", FileApi::strError(err));
    }
    return ret;
}

}

#endif
