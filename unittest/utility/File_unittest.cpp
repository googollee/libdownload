#include "utility/File.h"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

TEST(FileTest, NotOpenFile)
{
    File f;
    ASSERT_EQ(f.isOpen(), false);
}

TEST(FileTest, Create)
{
    File f;
    if (!f.open("./test.file", File::OF_Create | File::OF_Write))
    {
        // should not run here;
        ASSERT_TRUE(false);
    }

    ASSERT_EQ(f.isOpen(), true);

    ASSERT_EQ(f.close(), true);
    ASSERT_EQ(f.isOpen(), false);

    ASSERT_EQ(f.close(), false);
}

TEST(FileTest, ReadWrite)
{
    File f;
    f.open("./test.file", File::OF_Create | File::OF_Write);
    ASSERT_EQ(f.isOpen(), true);
    ASSERT_EQ(f.write("123", 3), 3);
    ASSERT_EQ(f.close(), true);
    ASSERT_EQ(f.isOpen(), false);

    f.open("./test.file", File::OF_Read);
    ASSERT_EQ(f.isOpen(), true);
    char buf[4];
    ssize_t got = f.read(buf, 4);
    ASSERT_EQ(got, 3);
    buf[got] = '\0';
    ASSERT_STREQ(buf, "123");

    f.close();
    ASSERT_EQ(f.isOpen(), false);
}

TEST(FileTest, SeekRead)
{
    File f;
    f.open("./test.file", File::OF_Read);
    ASSERT_EQ(f.isOpen(), true);

    ASSERT_EQ(f.seek(1, File::SF_FromBegin), true);

    char buf[4];
    ssize_t got = f.read(buf, 4);
    ASSERT_EQ(got, 2u);
    buf[got] = '\0';
    ASSERT_STREQ(buf, "23");

    f.close();
    ASSERT_EQ(f.isOpen(), false);
}

TEST(FileTest, CheckEof)
{
    File f;
    f.open("./test.file", File::OF_Read);
    ASSERT_EQ(f.isOpen(), true);
    ASSERT_EQ(f.isEof(), false);

    f.seek(1, File::SF_FromBegin);
    ASSERT_EQ(f.isEof(), false);

    char buf[4];
    f.read(buf, 4);
    ASSERT_EQ(f.isEof(), true);

    f.close();
    ASSERT_EQ(f.isOpen(), false);
}

TEST(FileTest, Resize)
{
    ASSERT_EQ(File::resize("./test.file", 10), true);

    File f;
    f.open("./test.file", File::OF_Read);
    f.seek(0, File::SF_FromEnd);
    ASSERT_EQ(f.tell(), 10u);
    f.close();

    File::resize("./test.file", 20);

    f.open("./test.file", File::OF_Read);
    f.seek(0, File::SF_FromEnd);
    ASSERT_EQ(f.tell(), 20u);
    f.close();
}

TEST(FileTest, Remove)
{
    ASSERT_EQ(File::exist("./test.file"), true);
    ASSERT_EQ(File::remove("./test.file"), true);
    ASSERT_EQ(File::exist("./test.file"), false);

    File f;
    ASSERT_EQ(f.open("./test.file", File::OF_Read), false);
}
