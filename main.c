#include "minunit.h"
#include "test/testsuite.h"

int main(int argc, char *argv[])
{
    MU_RUN_SUITE(vbr_test_suite);
    MU_RUN_SUITE(fat_test_suite);    
    MU_RUN_SUITE(cch_test_suite);
    MU_RUN_SUITE(cchdir_test_suite);

    return 0;
}
