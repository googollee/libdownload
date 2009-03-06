#ifndef DOWNLOAD_FILE_CLASS_HEAD
#define DOWNLOAD_FILE_CLASS_HEAD

#include <utility/Utility.h>

#include <stdio.h>

namespace filesystem
{

//using namespace boost::filesystem;

enum OpenFlag
{
    Read     = 0x1,
    Write    = 0x2,
    Create   = 0x4,
    Truncate = 0x8,
};

enum SeekFlag
{
    FromBegin   = 0,
    FromCurrent = 1,
    FromEnd     = 2,
};

}

#include "FilePosixApi.h"

namespace filesystem
{

template <typename T>
class FileBase : private Noncopiable
{
public:
    static void remove(const char *name);

    FileBase();
    FileBase(const char *name, int flag);
    ~FileBase();

    void open(const char *name, int flag);
    bool isOpen();
    void close();
    void resize(size_t len);

    size_t read(void *buffer, size_t max);
    size_t write(const void *buffer, size_t len);

    void seek(size_t pos, SeekFlag flag);
    bool seekWithLengthCheck(size_t pos);
    size_t tell();

private:
    T data;
};

typedef FileBase<FilePosixApi> File;

template <typename T>
inline File::FileBase<T>::FileBase()
{}

template <typename T>
inline File::FileBase<T>::FileBase(const char *name, int flag)
{
    if ( !data.open(name, flag) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline File::FileBase<T>::~FileBase()
{
    if (isOpen())
    {
        close();
    }
}

template <typename T>
inline void File::FileBase<T>::remove(const char *name)
{
    if ( !T::remove(name) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline void File::FileBase<T>::open(const char *name, int flag)
{
    if ( !data.open(name, flag) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline bool File::FileBase<T>::isOpen()
{
    return data.isOpen();
}

template <typename T>
inline void File::FileBase<T>::close()
{
    if ( !data.close() )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline void File::FileBase<T>::resize(size_t len)
{
    if ( !data.resize(len) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline size_t File::FileBase<T>::read(void *buffer, size_t max)
{
    size_t ret = 0;
    if ( !data.read(buffer, max, &ret) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
    return ret;
}

template <typename T>
inline size_t File::FileBase<T>::write(const void *buffer, size_t len)
{
    size_t ret = 0;
    if ( !data.write(buffer, len, &ret) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
    return ret;
}

template <typename T>
inline void File::FileBase<T>::seek(size_t pos, SeekFlag flag)
{
    if ( !data.seek(pos, flag) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
}

template <typename T>
inline bool File::FileBase<T>::seekWithLengthCheck(size_t pos)
{
    seek(0, FromEnd);
    if ( pos > tell() )
        return false;
    seek(pos, FromBegin);
    return true;
}

template <typename T>
inline size_t File::FileBase<T>::tell()
{
    size_t ret;
    if ( !data.tell(&ret) )
    {
        int err = T::getLastError();
        DOWNLOADEXCEPTION(err, "File", T::strError(err));
    }
    return ret;
}

}

#endif
