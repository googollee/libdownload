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
