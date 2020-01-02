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
    vv->debugging = 1;

    get_cpuid();

    const int kTestSizeDwords = SPINSZ_DWORDS * 2 + 508;

    // Allocate an extra cache line on each end, where we'll
    // write a sentinel pattern to detect overflow or underflow:
    const int kRawBufSizeDwords = kTestSizeDwords + 32;

    segs = 1;
    ulong raw_start = (ulong)malloc(kRawBufSizeDwords * sizeof(ulong));
    ulong raw_end = raw_start + kRawBufSizeDwords * sizeof(ulong);

    // align to a cache line:
    if (raw_start & 0x3f) {
        raw_start &= ~0x3f;
        raw_start += 0x40;
    }
    // align to a cache line:
    if (raw_end & 0x3f) {
        raw_end &= ~0x3f;
    }

    ulong start = raw_start + 64; // exclude low sentinel cache line
    ulong end   = raw_end   - 64; // exclude high sentinel cache line

    // setup sentinels
    memset((ulong*)raw_start, 'z', 64);
    memset((ulong*)end, 'z', 64);

    vv->map[0].start = (ulong*)start;
    vv->map[0].end = ((ulong*)end) - 1;  // map.end points to xxxxxfc

    const int iter = 2;
    const int me = 0;  // cpu ordinal

    // TEST 0
    // NOTE: in prod, this runs with cache disabled
    //       but we can't do anything about the cache in userspace.
    addr_tst1(me);

    // TEST 1, 2
    addr_tst2(me);

    // TEST 3, 4, 5, 6
    const ulong pat = 0x112211ee;
    movinv1(iter, pat, ~pat, 0);

    // TEST 7
    block_move(iter, me);

    // TEST 8
    movinv32(iter, 0x2, 0x1, 0x80000000, 0, 1, me);

    // TEST 9
    movinvr(me);

    // TEST 10
    modtst(2, 1, 0x5555aaaa, 0xaaaa5555, me);

    // TEST 11
    bit_fade_fill(0xdeadbeef, me);
    bit_fade_chk(0xdeadbeef, me);

#if 0
    // TODO: test the foreach routines with boundary address near 4G
    //
    // We can't malloc up there (it's where linux puts the stack)
    // but we can invent VAs up there so long as we don't attempt to access
    // them.
    vv->map[0].start = (ulong*)0xe0000000;
    vv->map[0].end   = (ulong*)0xfffffffc;
    movinv1(iter, pat, ~pat, 0);
#endif

    // Check sentinels, they should not have been overwritten. Do this last.
    for (int j=0; j<64; j++) {
        assert(((char*)raw_start)[j] == 'z');
        assert(((char*)end)[j]       == 'z');
    }

    return 0;
}
