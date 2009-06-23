#include "utility/Allocator.h"

#include <gtest/gtest.h>

using Utility::Allocator;

class Class
{
public:
    Class()
        {
            ++ctorCount;
        }

    ~Class()
        {
            ++dtorCount;
        }

    static int ctorCount;
    static int dtorCount;
};

int Class::ctorCount;
int Class::dtorCount;

TEST(AllocatorTest, Instance)
{
    Class::ctorCount = 0;
    Class::dtorCount = 0;

    Class *c = Allocator::alloc<Class>();
    ASSERT_EQ(Class::ctorCount, 1);
    ASSERT_EQ(Class::dtorCount, 0);

    Allocator::free(c);
    ASSERT_EQ(Class::ctorCount, 1);
    ASSERT_EQ(Class::dtorCount, 1);
}

TEST(AllocatorTest, Array)
{
    Class::ctorCount = 0;
    Class::dtorCount = 0;

    Class *c = Allocator::allocArray<Class>(10);
    ASSERT_EQ(Class::ctorCount, 10);
    ASSERT_EQ(Class::dtorCount, 0);

    Allocator::freeArray(c);
    ASSERT_EQ(Class::ctorCount, 10);
    ASSERT_EQ(Class::dtorCount, 10);
}
