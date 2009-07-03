#ifndef SOCKET_MANAGER_CLASS_HEAD
#define SOCKET_MANAGER_CLASS_HEAD

#ifdef SOCKET_MANAGER_TEST
#include <stdio.h>
#endif

#include "DownloadException.h"

namespace Utility
{

class SocketManager
{
public:
    static const int noLimited = -1;

    SocketManager(int maxOrphan, int maxConnect);
    ~SocketManager();

    bool get();
    void fail();
    void connect();
    void close();

#ifdef SOCKET_MANAGER_TEST

    int getOrphan();
    int getConnect();

#endif

private:
    int maxOrphan_;
    int maxConnect_;

    int orphan_;
    int connect_;
};

inline
SocketManager::SocketManager(int maxOrphan, int maxConnect)
    : maxOrphan_(maxOrphan),
      maxConnect_(maxConnect),
      orphan_(0),
      connect_(0)
{
    if (maxConnect_ < 0)
        maxConnect_ = noLimited;
    if (maxOrphan_ < 0)
        maxOrphan_ = noLimited;
}

inline
SocketManager::~SocketManager()
{}

inline
bool SocketManager::get()
{
    if (maxOrphan_ == noLimited && maxConnect_ == noLimited)
    {
        return true;
    }

    if (orphan_ >= maxOrphan_)
    {
        return false;
    }

    if (maxConnect_ != noLimited && (orphan_ + connect_) >= maxConnect_)
    {
        return false;
    }

    ++orphan_;

    return true;
}

inline
void SocketManager::fail()
{
    if (maxOrphan_ == noLimited)
        return;

    if (orphan_ < 1)
        DOWNLOADEXCEPTION(1, "SocketManager", "orphan < 1 in fail()");

    --orphan_;
}

inline
void SocketManager::connect()
{
    if (maxOrphan_ == noLimited)
        return;

    if (orphan_ < 1)
        DOWNLOADEXCEPTION(2, "SocketManager", "orphan < 1 in connect()");

    --orphan_;

    if (maxConnect_ == noLimited)
        return;

    if ((connect_ + orphan_) >= maxConnect_)
        DOWNLOADEXCEPTION(3, "SocketManager", "connect >= max connect in connect()");

    ++connect_;
}

inline
void SocketManager::close()
{
    if (maxConnect_ == noLimited)
        return;

    if (connect_ < 1)
        DOWNLOADEXCEPTION(4, "SocketManager", "orphan < 1 in connect()");

    --connect_;
}

#ifdef SOCKET_MANAGER_TEST

inline
int SocketManager::getOrphan()
{
    return orphan_;
}

inline
int SocketManager::getConnect()
{
    return connect_;
}

#endif

}

#endif
