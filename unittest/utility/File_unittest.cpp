#include "utility/File.h"

#include <gtest/gtest.h>

using FileWrapper::File;

TEST(FileTest, Create)
{
    File f("./test.file", FileWrapper::Create | FileWrapper::Write);
    f.close();
}

TEST(FileTest, Resize)
{
    File f("./test.file", FileWrapper::Create | FileWrapper::Write);
    f.resize(10);
    f.close();
}
