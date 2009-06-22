#define SOCKET_MANAGER_TEST

#include "utility/SocketManager.h"

#include <gtest/gtest.h>

#include <time.h>

using Utility::SocketManager;

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

TEST(SocketManagerTest, RandomTest)
{
    const int max = 1000;
    srand( time(NULL) );
    SocketManager m(5, 10);

    for (int i=0; i< max; ++i)
    {
        switch (rand() % 4)
        {
        case 0:
        {
            int orphan = m.getOrphan() + 1;
            int connect = m.getConnect();
            bool ret = m.get();
            if (orphan <= 5 && (orphan + connect) <= 10)
            {
                if (!ret)
                {
                    printf("get() false while orphan = %d, connect = %d\n", m.getOrphan(), m.getConnect());
                }
                ASSERT_EQ(ret, true);
            }
            else
            {
                if (ret)
                {
                    printf("get() true while orphan = %d, connect = %d\n", m.getOrphan(), m.getConnect());
                }
                ASSERT_EQ(ret, false);
            }
            break;
        }
        case 1:
            m.fail();
            break;
        case 2:
            m.connect();
            break;
        case 3:
            m.close();
            break;
        }
    }
}
