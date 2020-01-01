/* Usermode self-test.
 *
 * This is a unit test for memtest86+ itself,
 * not a standalone usermode memory tester. Sorry.
 *
 * It covers the main test routines in test.c, allows running them
 * in a debugger or adding printfs as needed.
 *
 * It does not cover:
 *  - SMP functionality. There's no fundamental obstacle
 *    to this, it's just not here yet.
 *  - Every .c file. Some of them do things that will be
 *    difficult to test in user mode (eg set page tables)
 *    without some refactoring.
 */

#ifdef NDEBUG
 #error "Someone disabled asserts, but we want asserts for the self test."
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdint.h"
#include "cpuid.h"
#include "test.h"

/* Provide alternate versions of the globals */
volatile int run_cpus = 1;
volatile int bail = 0;
volatile int segs = 0;
struct vars variables;
struct vars* const vv = &variables;
// Self-test only supports single CPU (ordinal 0) for now:
volatile int mstr_cpu = 0;

void assert_fail(const char* file, int line_no) {
    printf("Failing assert at %s:%d\n", file, line_no);
    assert(0);
}

void do_tick(int me) {}
void hprint(int y, int x, ulong val) {}
void cprint(int y,int x, const char *s) {}
void dprint(int y,int x,ulong val,int len, int right) {}
void s_barrier() {}   // No SMP support in selftest yet

// Selftest doesn't have error injection yet, and thus, we
// never expect to detect any error. Fail an assert in these
// error-reporting routines:
void ad_err1(ulong *adr1, ulong *adr2, ulong good, ulong bad) {
    assert(0);
}
void ad_err2(ulong *adr, ulong bad) {
    assert(0);
}
void mt86_error(ulong* adr, ulong good, ulong bad) {
    assert(0);
}

int main() {
    memset(&variables, 0, sizeof(variables));

    int stack_var;
    int* heap_var = malloc(sizeof(int));
    printf("Stack = %p, heap = %p\n", &stack_var, heap_var);
    free(heap_var);

    get_cpuid();
    vv->debugging = 1;

    const int kTestSizeDwords = SPINSZ_DWORDS * 2 + 512;

    segs = 1;
    vv->map[0].start = malloc(kTestSizeDwords * sizeof(ulong));
    vv->map[0].end = vv->map[0].start + kTestSizeDwords - 1;

    const int iter = 1;

    const ulong pat = 0x112211ee;
#if 0
    movinv1(iter, pat, ~pat, 0);

#else
    // What we unfortunately can't test easily in userspace,
    // that we need to test, is VA's aligned near the top of 4G.
    //
    // I mean, we can't malloc up there (it's where linux puts the stack)
    // but we can invent VAs up there and run a cut-down routine
    // that doesn't actually touch memory but only reports what it would
    // have done.
    vv->map[0].start = (ulong*)0xe0000000;
    vv->map[0].end   = (ulong*)0xfffffffc;
    movinv1(iter, pat, ~pat, 0);
#endif

    return 0;
}