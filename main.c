#include "minunit.h"
#include "test/testsuite.h"

int main(int argc, char *argv[])
{
    MU_RESET();

    MU_RUN_SUITE(vbr_test_suite);
    MU_RUN_SUITE(fat_test_suite);
    MU_RUN_SUITE(cluster_chain_test_suite);

    MU_REPORT();

    return 0;
}
