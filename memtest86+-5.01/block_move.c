#include "test.h"
#include "stdint.h"
#include "cpuid.h"
#include "smp.h"

typedef struct {
    int iter;
    int me;
} block_move_ctx;

extern volatile int bail;

void sliced_foreach_segment
(const void *ctx, int me, segment_fn func);

void block_move_init(ulong* restrict buf,
                     ulong len_dw, const void* unused_ctx);
void block_move_move(ulong* restrict buf,
                     ulong len_dw, const void* vctx);
void block_move_check(ulong* restrict buf,
                      ulong len_dw, const void* unused_ctx);

#pragma GCC push_options
#pragma GCC optimize ("O0")

/*
 * Test memory using block moves 
 * Adapted from Robert Redelmeier's burnBX test
 */
void block_move(int iter, int me)
{
    // BOZO
    //    cprint(LINE_PAT, COL_PAT-2, "          ");

    block_move_ctx ctx;
    ctx.iter = iter;
    ctx.me = me;

    // JPC BOZO:
    // Try with only the first 2 foreachs
    // and then with only the first 1
    //
    // Conclusion: if the block_move_move call
    // is unoptimized or iffed-out, we're fine,
    // and if optimized we crash.
    //
    // Everything we've done so far is with inlining
    // enabled (I think) but can we disable that, and thus
    // ensure the push/pop of optimizations really applies
    // exactly where it says? Try that...
    //  * -fno-inline-small-functions had no effect
    //    on the generated assembly
    //
    // It doesn't look like the difference between optimized
    // and unoptimized block_move() -- and between pass and fail
    // -- is anything getting inlined. The functions are too close
    // in length, probably.

    // What about this: if we nop movsl out, does it crash?
    // Rule a collision there in or out...
    // WHOA: we still crash without the movsl. huh!!


    // THINGS TO TRY:
    //  - zero out the rewritten addr_tst1 in case that's toxic
    //    (it's not)
    //  - zero out the block_move_init in case that's toxic
    //    (it's not)


    // Regroup, 1/4
    // * The crash happens on bare metal always, but never vbox,
    //   why? Also the crash has varying end state PC's, why?
    //   - Could be a function of e820 layout (likely)
    //   - Could be that bare metal has unpredictable initial memory
    //     state? IWBN if vbox could mimic that...
    //     Running a bit to garbage up memory, and then tapping
    //     ESC could work? Except this leads to an instant crash
    //     on restart (not in test 7, but first thing)
    //     * with block_move() optimized: this new crash happens
    //     * with block_move() unopt: this new crash still happens
    //       so it's a different failure mode with a different
    //       cause (could be related to T400 startup failure?)
    //
    // * Is it s_barrier()? Why do we have s_barrier() calls in
    //   this test but not any other?
    //   - Opt block_move, s_barrier() definition gutted: crash
    //   - Opt block_move, s_barrier() calls commented:
    //      new failure mode -- test runs and reports many errors
    //      below 1MB address.
    //   - Unopt block_move, s_barrier() calls commented: no crash
    //
    // * One idea is to produce the smallest function that behaves
    //   different between opt and noopt and then scrutinize the
    //   assembly carefully (for a compiler bug? maybe?)
    //   - that works if the failure mode is a crash, if we convert
    //     the failure to a reported memory error, we can't cut out
    //     the block_move_check() step. hmmm. maybe go back to
    //     full crash failure mode? (also you kind of know better
    //     what to look for in bad assembly, maybe, with the harder
    //     crash?)

    // * Another idea:
    //   - There are two differences between opt and noopt:
    //     It's a different instruction stream, and different
    //     addresses for many other functions. Can we separate
    //     those effects?

    // * How did I not notice this before:
    //   - without the p2aligns, and with noopt, 'block_move' is
    //     the first symbol in test.o
    //     with opt, it falls to an address over 0x1000
    //   - so, we'll have to move block_move into its own TU
    //     checkpoint the code first.

    // After moving block_move into block_move.c :
    //  * we can still toggle the crash by toggling the O0 pragma.
    //  * -falign-functions=64 yields identical block_move lengths
    //     but only when trailing function fooff() is present
    //  * Q) Did we really produce binaries that are identical
    //       apart from the block_move instruction stream?
    //    A1) No, because the linker is too smart and prunes
    //        the padding back out (facepalm)
    //    A2) No, because the GOT is a different
    //        size (or diff position?) for opt vs noopt cases.
    //        This should be independent of the size of block_move.
    //        Confirmed that both block_moves have equal sets of
    //        linker-resolved addresses.
    //    A3) Why do we even get an .eh_frame section, and why
    //        is it completely different for opt vs noopt?

    // Ugh, let's just find the shortest thing that'll crash
    //  * opt, remove cprint, remove block_move_check: crash
    //  * opt, remove both BAILRs: crash
    //  * opt, remove the block_move_init and its s_barrier: crash
    //    - noopt of same: no crash
    //  * opt, remove the block_move_move, retain its s_barrier:
    //    - no crash and runs extremely fast
    //    - this is cutting too far

#if 0
    /* Initialize memory with the initial pattern.  */
    sliced_foreach_segment(&ctx, me, block_move_init);
    //{ BAILR }
    s_barrier();
#endif

    /* Now move the data around */
    sliced_foreach_segment(&ctx, me, block_move_move);
    //{ BAILR }
    s_barrier();

#if 0  // removing this doesn't fix the crash
    /* And check it. */
    sliced_foreach_segment(&ctx, me, block_move_check);
#endif  // this does not move
}

#pragma GCC pop_options

// Must be present for -falign-functions to have any effect
// (BOZO that seems like a compiler bug by itself!)
void fooff() {}
