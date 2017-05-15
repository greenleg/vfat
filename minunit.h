#ifndef __MINUNIT_H__
#define __MINUNIT_H__

#include <unistd.h>	      /* POSIX flags */
#include <time.h>	      /* clock_gettime(), time() */
#include <sys/time.h>	  /* gettimeofday() */
#include <sys/resource.h>
#include <sys/times.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/*  Maximum length of last message */
#define MINUNIT_MESSAGE_LEN 4096

/*  Accuracy with which floats are compared */
#define MINUNIT_EPSILON 1E-12

#define UNKNOWN_CLOCK_ID (clockid_t) -1
#define UNKNOWN_CLOCK (clock_t) -1

/*  Misc. counters */
static int minunit_run = 0;
static int minunit_assert = 0;
static int minunit_fail = 0;
static int minunit_status = 0;

/*  Timers */
static double minunit_real_timer = 0;
static double minunit_cpu_timer = 0;

/*  Last message */
static char minunit_last_message[MINUNIT_MESSAGE_LEN];

/*  Definitions */
#define MU_TEST(method_name) static void method_name()

#define MU__SAFE_BLOCK(block) do {\
    block\
} while(0)

/*  Reset counters and timers */
#define MU_RESET() MU__SAFE_BLOCK(\
    minunit_run = 0;\
    minunit_assert = 0;\
    minunit_fail = 0;\
    minunit_status = 0;\
    minunit_real_timer = mu_timer_real();\
    minunit_cpu_timer = mu_timer_cpu();\
)

/*  Test runner */
#define MU_RUN_TEST(test) MU__SAFE_BLOCK(\
    minunit_status = 0;\
    test();\
    ++minunit_run;\
    if (minunit_status) {\
        ++minunit_fail;\
        printf("F\n%s\n", minunit_last_message);\
    }\
    fflush(stdout);\
)

#define MU_PRINT_TEST_INFO() MU__SAFE_BLOCK(\
    printf("[MU_TEST] %s %s\n", __FILE__, __func__);\
)

/*  Report */
#define MU_REPORT() MU__SAFE_BLOCK(\
    printf("\n\n%d tests, %d assertions, %d failures\n", minunit_run, minunit_assert, minunit_fail);\
    double minunit_end_real_timer = mu_timer_real();\
    double minunit_end_cpu_timer = mu_timer_cpu();\
    printf("\nFinished in %.8f seconds (real) %.8f seconds (proc)\n\n",\
        minunit_end_real_timer - minunit_real_timer,\
        minunit_end_cpu_timer - minunit_cpu_timer);\
)

/*  Assertions */
#define MU_ASSERT(test) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    if (!(test)) {\
        snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d:", __func__, __FILE__, __LINE__);\
        minunit_status = 1;\
        return;\
    } else {\
        minunit_status = 0;\
    }\
)

#define MU_ASSERT_MSG(test, message) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    if (!(test)) {\
        snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %s", __func__, __FILE__, __LINE__, message);\
        minunit_status = 1;\
        return;\
    } else {\
        minunit_status = 0;\
    }\
)

#define MU_FAIL(message) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %s", __func__, __FILE__, __LINE__, message);\
    minunit_status = 1;\
    return;\
)

#define MU_ASSERT_INT_EQ(expected, actual) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    int minunit_tmp_e = (expected);\
    int minunit_tmp_a = (actual);\
    if (minunit_tmp_e != minunit_tmp_a) {\
        snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %d expected but was %d", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_a);\
        minunit_status = 1;\
        return;\
    } else {\
        minunit_status = 0;\
    }\
)

#define MU_ASSERT_DOUBLE_EQ(expected, result) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    double minunit_tmp_e = (expected);\
    double minunit_tmp_r = (result);\
    if (fabs(minunit_tmp_e - minunit_tmp_r) > MINUNIT_EPSILON) {\
        int minunit_significant_figures = 1 - log10(MINUNIT_EPSILON);\
        snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: %.*g expected but was %.*g", __func__, __FILE__, __LINE__, minunit_significant_figures, minunit_tmp_e, minunit_significant_figures, minunit_tmp_r);\
        minunit_status = 1;\
        return;\
    } else {\
        minunit_status = 0;\
    }\
)

#define MU_ASSERT_STRING_EQ(expected, result) MU__SAFE_BLOCK(\
    ++minunit_assert;\
    const char* minunit_tmp_e = (expected);\
    const char* minunit_tmp_r = (result);\
    if (!minunit_tmp_e) {\
        minunit_tmp_e = "<null>";\
    }\
    if (!minunit_tmp_r) {\
        minunit_tmp_r = "<null>";\
    }\
    if(strcmp(minunit_tmp_e, minunit_tmp_r)) {\
        snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN, "%s failed:\n\t%s:%d: '%s' expected but was '%s'", __func__, __FILE__, __LINE__, minunit_tmp_e, minunit_tmp_r);\
        minunit_status = 1;\
        return;\
    } else {\
        minunit_status = 0;\
    }\
)

/*
 * The following two functions were written by David Robert Nadeau
 * from http://NadeauSoftware.com/ and distributed under the
 * Creative Commons Attribution 3.0 Unported License
 */

/**
 * Returns the real time, in seconds, or -1.0 if an error occurred.
 *
 * Time is measured since an arbitrary and OS-dependent start time.
 * The returned real time is only useful for computing an elapsed time
 * between two calls to this function.
 */
static double mu_timer_real()
{
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)    
    {
        struct timespec ts;
#if defined(CLOCK_MONOTONIC_RAW)        
        const clockid_t id = CLOCK_MONOTONIC_RAW;
#else
        const clockid_t id = UNKNOWN_CLOCK_ID;
#endif
        if ( id != UNKNOWN_CLOCK_ID && clock_gettime( id, &ts ) != -1 ) {
            return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
        }
    }
#endif

    struct timeval tm;
    if (gettimeofday( &tm, NULL ) != -1) {
        return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
    }

    return -1.0;
}

/**
 * Returns the amount of CPU time used by the current process,
 * in seconds, or -1.0 if an error occurred.
 */
static double mu_timer_cpu()
{
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)    
    {
        clockid_t id;
        struct timespec ts;
#if _POSIX_CPUTIME > 0
        /* Clock ids vary by OS. Query the id, if possible. */
        if ( clock_getcpuclockid( 0, &id ) == -1 )
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
            /* Use known clock id for AIX, Linux, or Solaris. */
            id = CLOCK_PROCESS_CPUTIME_ID;
#else
            id = UNKNOWN_CLOCK_ID;
#endif
        if ( id != UNKNOWN_CLOCK_ID && clock_gettime( id, &ts ) != -1 ) {
                return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
        }
    }
#endif

#if defined(RUSAGE_SELF)
    {
        struct rusage rusage;
        if ( getrusage( RUSAGE_SELF, &rusage ) != -1 ) {
            struct timeval utime = rusage.ru_utime;
            return (double)utime.tv_sec + (double)utime.tv_usec / 1000000.0;
        }
    }
#endif

#if defined(_SC_CLK_TCK)
    {
        const long int ticks = sysconf( _SC_CLK_TCK );
        struct tms tms;
        if ( times( &tms ) != UNKNOWN_CLOCK ) {
            return (double)tms.tms_utime / (double)ticks;
        }
    }
#endif

#if defined(CLOCKS_PER_SEC)
    {
        clock_t cl = clock();
        if ( cl != UNKNOWN_CLOCK ) {
            return (double)cl / (double)CLOCKS_PER_SEC;
        }
    }
#endif

    return -1.0;
}

#endif /* __MINUNIT_H__ */
