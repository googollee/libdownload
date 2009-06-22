#ifndef SOCKET_MANAGER_CLASS_HEAD
#define SOCKET_MANAGER_CLASS_HEAD

#ifdef SOCKET_MANAGER_TEST
#include <stdio.h>
#endif

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
#ifdef SOCKET_MANAGER_TEST
        printf("no limited\n");
#endif
        return true;
    }

    if (orphan_ >= maxOrphan_)
    {
#ifdef SOCKET_MANAGER_TEST
        printf("orphan_ %d >= maxOrphan %d\n", orphan_, maxOrphan_);
#endif
        return false;
    }

    if (maxConnect_ != noLimited && (orphan_ + connect_) >= maxConnect_)
    {
#ifdef SOCKET_MANAGER_TEST
        printf("(orphan_ %d + connect_ %d) >= maxConnect_ %d\n", orphan_, connect_, maxConnect_);
#endif
        return false;
    }

    ++orphan_;

    return true;
}

inline
void SocketManager::fail()
{
    if (maxOrphan_ != noLimited && orphan_ > 0)
        --orphan_;
}

inline
void SocketManager::connect()
{
    if (maxOrphan_ != noLimited && orphan_ > 0)
        --orphan_;

    if (maxConnect_ != noLimited && connect_ < maxConnect_)
        ++connect_;
}

inline
void SocketManager::close()
{
    if (maxConnect_ != noLimited && connect_ > 0)
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
