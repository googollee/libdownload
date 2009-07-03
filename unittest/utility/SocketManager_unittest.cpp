#define SOCKET_MANAGER_TEST

#include "utility/SocketManager.h"
#include "utility/DownloadException.h"

#include <gtest/gtest.h>

#include <time.h>

#include <vector.h>

using Utility::SocketManager;
using Utility::DownloadException;

TEST(SocketManagerTest, NoLimited)
{
    SocketManager m(SocketManager::noLimited, SocketManager::noLimited);

    for (int i=0; i<10; ++i)
    {
        ASSERT_EQ(m.get(), true);
    }
}

TEST(SocketManagerTest, OrphanTest)
{
    SocketManager m(5, 10);

    for (int i=0; i<5; ++i)
    {
        ASSERT_EQ(m.get(), true);
    }

    for (int i=0; i<5; ++i)
    {
        ASSERT_EQ(m.get(), false);
    }

    for (int i=0; i<3; ++i)
    {
        m.connect();
    }

    for (int i=0; i<3; ++i)
    {
        ASSERT_EQ(m.get(), true);
    }

    for (int i=0; i<5; ++i)
    {
        ASSERT_EQ(m.get(), false);
    }
}

TEST(SocketManagerTest, ConnectTest)
{
    SocketManager m(5, 10);

    for (int i=0; i<10; ++i)
    {
        ASSERT_EQ(m.get(), true);
        m.connect();
    }

    for (int i=0; i<5; ++i)
    {
        ASSERT_EQ(m.get(), false);
    }
}

class StubSocket
{
public:
    enum Status
    {
        S_CLOSED,
        S_CONNECTING,
        S_CONNECTED,
    };

    StubSocket(SocketManager &mng)
        : mng_(mng),
          status_(S_CLOSED)
        {}

    StubSocket(const StubSocket &arg)
        : mng_(arg.mng_),
          status_(arg.status_)
        {}

    StubSocket operator=(const StubSocket &arg)
        {
            if (&arg == this)
                return *this;

            this->mng_ = arg.mng_;
            this->status_ = arg.status_;

            return *this;
        }

    void randNext()
        {
            int s = rand() % 3;
            switch (status_)
            {
            case S_CLOSED:
                switch (s)
                {
                case 1:
                case 2:
                    if (mng_.get())
                        status_ = S_CONNECTING;
                    break;
                case 0:
                default:
                    break;
                }
                break;
            case S_CONNECTING:
                switch (s)
                {
                case 1:
                    mng_.fail();
                    status_ = S_CLOSED;
                    break;
                case 2:
                    mng_.connect();
                    status_ = S_CONNECTED;
                    break;
                case 0:
                default:
                    break;
                }
                break;
            case S_CONNECTED:
                switch (s)
                {
                case 1:
                case 2:
                    mng_.close();
                    status_ = S_CLOSED;
                    break;
                case 0:
                default:
                    break;
                }
                break;
            }
        }

private:
    SocketManager &mng_;
    Status status_;
};

TEST(SocketManagerTest, RandomTest)
{
    const int maxSocket  = 1000;
    const int maxStep    = 1000000;
    const int maxOrphan  = 10;
    const int maxConnect = 100;
    srand( time(NULL) );
    SocketManager m(maxOrphan, maxConnect);
    std::vector<StubSocket> sockets;

    for (int i=0; i<maxSocket; ++i)
    {
        StubSocket s(m);
        sockets.push_back(s);
    }

    try
    {
        for (int i=0; i<maxStep; ++i)
        {
            printf("%d%%\r", i / (maxStep/100));
            for (int j=0; j<maxSocket; ++j)
            {
                sockets[j].randNext();
            }
            ASSERT_TRUE((m.getOrphan() <= maxOrphan) && (m.getConnect() <= maxConnect));
        }
    }
    catch (DownloadException &e)
    {
        printf("orphan = %d, connect = %d\n", m.getOrphan(), m.getConnect());
        printf("error: %s in %s\n", e.what(), e.component());
        ASSERT_TRUE(false);
    }
}
