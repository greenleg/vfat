#include "minunit.h"
#include "test/testsuite.h"

int main(int argc, char *argv[])
{
    MU_RESET();

    run_vbr_suite();

    MU_REPORT();

    return 0;
}
