#include "utility/LocalPtr.h"

#include <gtest/gtest.h>

using Utility::LocalPtr;

int check = 0;

struct Sth
{
};

Sth* create_sth()
{
    ++check;
    return new Sth();
}

void free_sth(Sth* sth)
{
    --check;
    delete sth;
}

int checkFunc(bool quit)
{
    LocalPtr<Sth> sth(create_sth(), &free_sth);

    if (quit)
        return -1;

    return 0;
}

TEST(LocalPtrTest, Normal)
{
    ASSERT_EQ(checkFunc(false), 0);
    ASSERT_EQ(check, 0);
}

TEST(LocalPtrTest, Quit)
{
    ASSERT_EQ(checkFunc(true), -1);
    ASSERT_EQ(check, 0);
}
