#include <iostream>

//#include "test/minunit.h"
//#include "test/testsuite.h"
#include "gtest/gtest.h"

using namespace std;

int main(int argc, char *argv[])
{
//    MU_RUN_SUITE(vbr_test_suite);
//    MU_RUN_SUITE(fat_test_suite);
//    MU_RUN_SUITE(cch_test_suite);
//    MU_RUN_SUITE(cchdir_test_suite);
//    MU_RUN_SUITE(cchfile_test_suite);
//    MU_RUN_SUITE(filesys_test_suite);

    //::testing::GTEST_FLAG(filter) = "TestFixture1.Test1";
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}
