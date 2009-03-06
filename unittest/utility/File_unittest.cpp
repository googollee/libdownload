#include "utility/File.h"

#include <gtest/gtest.h>

namespace fs=filesystem;

TEST(FileTest, Create)
{
    fs::File f("./test.file", fs::Create | fs::Write);
    f.close();
}

TEST(FileTest, Resize)
{
    fs::File f("./test.file", fs::Create | fs::Write);
    f.resize(10);
    f.close();
}

TEST(FileTest, Remove)
{
    try
    {
        fs::File::remove("./test.file");
    }
    catch (DownloadException &e)
    {
        printf("catch exception: %s:%u error %d in %s: %s\n",
               e.file(), e.lineoff(),
               e.error(), e.component(), e.what());
    }
}
