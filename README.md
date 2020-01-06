# memtest

Fork of memtest86+ 5.01. Improves its correctness, code health, and code
readability while doing no harm to performance and coverage.

Non-goals of this fork:
 - Support UEFI
 - Build as a 64-bit binary
 - Improve SMP support. It's still experimental as always. I have done most
   testing in single core mode.


## TODO

 - Apply the rest of the ubuntu/debian patches.
   Get in touch with distro maintainers. what's the next step?

 - Attempt to update reloc.c from a recent glibc that knows how
   to relocate more ELF section types. I suspect we aren't completely
   relocating everything we need to; see comment on the STATIC
   macro defined in test.c

 - Test SMP a little. Getting SMP fully stable is not a goal, it's
   still experimental, but let's not make it worse.
   * works fine in VBox
   * it's not the reloc.c assert, removing that doesn't help
   * try removing addr_tst1 and also the sole ASSERT
     in main.c
     - no luck, still hangs
   * try with the old (5.01) test.c
     since its API hasn't changed at all
     - no luck, still hangs
     - that reduces the search space a lot
       (assuming 5.01 stock still works)



 - Update the version number to 6.0. Where does the 5.01 print from?


## Changes since 5.01

 - Specify units on major APIs, data structures, and shared constants.
   Never again wonder whether a given integer is a VA, PA, byte address,
   dword index, 4k page index, and so on.

 - Add an ASSERT() macro for validating assumptions. Prints a message
   in yellow (not red) to distinguish asserts from memory errors.

 - Add a unit test for the core memtest86+ routines in test.c. The
   "self_test" allows running these routines in gdb or adding debug
   prints. Prod code can use the DEBUGF macro to print messages.

   The unit test does not yet support SMP or error injection. It does
   cover all the major test routines. You need a 32-bit libc (sorry.)

 - Clean up the duplicative and brittle loop tests in test.c. There were
   address-dependent bugs here, eg. they would only manifest if a segment
   ended at exactly 4G or had a length that was a near multiple of SPINSZ.

   Some bugs were silent, for example test 7 would stop early (without
   testing all memory) if the range from calculate_chunk() maps to the
   exact top of 4GB address space. Other tests may have the same bug.

   To deny bugs their habitat, all tests share a new 'foreach_segment()'
   routine providing the segment-loop logic in a single central place.
   The new logic avoids overflow, not by being excruciatingly careful,
   but by computing loop bounds in terms of dword indices instead of
   byte addresses. Thus all quantities are far from the 4G overflow line.

 - Avoid 1- and 2-letter variable names. This is a work in progress.
   Favor names that are meaningful, or at least searchable.

 - Indent consistently. Replace tab characters with spaces so the code will
   render the same for everyone (the jwz standard.)

 - In test 7 (block moves) replace assembly with C for everything except
   the 'movsl' instruction that does all the real work of this test.
   The rewritten test performs within a measurement error of the original.
   I have a flaky machine which fails the original block_move test, and it
   also fails the new one with the same failure mode, as expected.

 - In test 10 (modtst) replace assembly with C. It had to be modified to
   work with the new loop bounds logic. I was less confident in my ability
   to modify assembly. GCC's output looks just as tight as the handwritten
   assembly.

   Side note: we're compiling test.c with -O1 now. Someone had knocked it
   down to -O0 in 5.01 (why? to make the assembly look better? to
   work around one of the many bugs that are now fixed?) In any event,
   things are working well at -O1 now, which helps the compiler to
   compete with manual assembly.

 - Rewrite test 1 (addr_tst1) to fix a bug. Before, it could possibly
   write to stray addresses that aren't mapped to DRAM -- and could be
   mapped to hardware devices, uh oh! The simplified test combines
   the unbanked and banked modes, since the banked mode pretty much
   covered the unbanked mode. It uses the foreach_segment() routine
   and dword-indexed-based loop tests to avoid overflows.
