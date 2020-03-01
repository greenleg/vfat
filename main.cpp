#include <iostream>

//#include "test/minunit.h"
//#include "test/testsuite.h"
#include "gtest/gtest.h"

using namespace std;

#ifndef RUN_ALL_TEST
int main(int argc, char *argv[])
{
////    MU_RUN_SUITE(vbr_test_suite);
////    MU_RUN_SUITE(fat_test_suite);
////    MU_RUN_SUITE(cch_test_suite);
////    MU_RUN_SUITE(cchdir_test_suite);
////    MU_RUN_SUITE(cchfile_test_suite);
////    MU_RUN_SUITE(filesys_test_suite);
    return 0;
}
#endif

// Tests positive input.
TEST(TestFixture1, Test1) {
    EXPECT_EQ(1, 1);
}

// Tests positive input.
TEST(TestFixture1, Test2) {
    EXPECT_EQ(1, 2);
}
