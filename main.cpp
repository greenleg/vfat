#include <iostream>

#include "test/minunit.h"
#include "test/testsuite.h"


using namespace std;

int main()
{
    MU_RUN_SUITE(vbr_test_suite);
//    MU_RUN_SUITE(fat_test_suite);
//    MU_RUN_SUITE(cch_test_suite);
//    MU_RUN_SUITE(cchdir_test_suite);
//    MU_RUN_SUITE(cchfile_test_suite);
//    MU_RUN_SUITE(filesys_test_suite);

    cout << "Finished." << endl;

    return 0;
}
