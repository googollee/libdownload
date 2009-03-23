#include "utility/File.h"

#include <gtest/gtest.h>

TEST(FileTest, Create)
{
    File f("./test.file", OF_Create | OF_Write);
    f.close();
}

TEST(FileTest, Resize)
{
    File f("./test.file", OF_Create | OF_Write);
    f.resize(10);
    f.close();
}

TEST(FileTest, Remove)
{
    try
    {
        File::remove("./test.file");
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
    }
}
