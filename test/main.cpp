#include <iostream>
#include "gtest/gtest.h"

using namespace std;

int main(int argc, char *argv[])
{
    //::testing::GTEST_FLAG(filter) = "BootRecordTest.FileDisk";
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}