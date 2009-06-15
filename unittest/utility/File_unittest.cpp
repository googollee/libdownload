#include "utility/File.h"

#include <gtest/gtest.h>

#include <string>

using Utility::File;

TEST(FileTest, NotOpenFile)
{
    try
    {
        File f;
        ASSERT_EQ(f.isOpen(), false);
    }
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, Create)
{
    try
    {
        File f;
        f.open("./test.file", File::OF_Create | File::OF_Write);
        ASSERT_EQ(f.isOpen(), true);

        f.close();
        ASSERT_EQ(f.isOpen(), false);
    }
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, ReadWrite)
{
    try
    {
        File f;
        f.open("./test.file", File::OF_Create | File::OF_Write);
        ASSERT_EQ(f.isOpen(), true);
        f.write("123", 3);
        f.close();
        ASSERT_EQ(f.isOpen(), false);

        f.open("./test.file", File::OF_Read);
        ASSERT_EQ(f.isOpen(), true);
        char buf[4];
        size_t got = f.read(buf, 4);
        ASSERT_EQ(got, 3u);
        buf[got] = '\0';
        f.close();
        ASSERT_EQ(f.isOpen(), false);

        std::string str(buf, 3);
        ASSERT_EQ(str, "123");
    }
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, SeekRead)
{
    try
    {
        File f;
        f.open("./test.file", File::OF_Read);
        ASSERT_EQ(f.isOpen(), true);
        f.seek(1, File::SF_FromBegin);
        char buf[4];
        size_t got = f.read(buf, 4);
        ASSERT_EQ(got, 2u);
        buf[got] = '\0';
        f.close();
        ASSERT_EQ(f.isOpen(), false);

        ASSERT_STREQ(buf, "23");
    }
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, CheckEof)
{
    try
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
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, Resize)
{
    try
    {
        File::resize("./test.file", 10);

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
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());

        throw;
    }
}

TEST(FileTest, Remove)
{
    try
    {
        File::remove("./test.file");
    }
    catch (Utility::DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
        throw;
    }

    try
    {
        File f;
        f.open("./test.file", File::OF_Read);

        // should not run here;
        ASSERT_TRUE(false);
    }
    catch (Utility::DownloadException &e)
    {
    }
}
