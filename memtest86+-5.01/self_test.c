/* Usermode self-test.
 *
 * Covers the main test routines in test.c, allows running them
 * in a debugger or adding printfs as needed.
 *
 * The intent of this is to act as a unit test for memtest86+ itself,
 * not to be a standalone usermode memory tester.
 */

#include <assert.h>
#include <stdio.h>

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
void ad_err1(ulong *adr1, ulong *adr2, ulong good, ulong bad) {}
void ad_err2(ulong *adr, ulong bad) {}
void s_barrier() {}   // No SMP support in selftest yet!

int main() {
    // TODO: init 'segs', 'vv->map'
    get_cpuid();

    const int iter = 1;


    const ulong pat = 0x112211ee;
    movinv1(iter, pat, ~pat, 0);

    return 0;
}
