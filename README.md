# memtest

Fork of memtest86+ 5.01. Improves its correctness, code health, and code
readability while doing no harm to performance and coverage.

Non-goals of this fork:
 - Support UEFI
 - Build as a 64-bit binary
 - Improve SMP support. It's still experimental as always. I have done most
   testing in single core mode.

## TODO

 - Review to see if I've broken any of the ticks-counting logic,
   now that I see how fragile it is (doh!)

 - Apply the rest of the ubuntu/debian patches.
   Get in touch with distro maintainers. what's the next step?

 - Test SMP a little bit, ensure it's not borked. Getting SMP fully
   stable is not a goal but let's not make it worse.

 - Update the version number to 6.0. (Where does the 5.01 print from??)

## Changes since 5.01

 - Specify units on major APIs, data structures, and magic-number
   macros. Use comments and naming to clarify whether a given
   integer represents a physical address, virtual address,
   dword index, cache line index, 4k page number, etc.

 - Add an ASSERT() macro for testing assumptions and catching coding
   bugs. Prints a message in yellow (not red) since assertion failures
   are not usually memory errors.

 - Add a usermode self_test. This is not a memory test, it's a unit test
   for the core memtest86+ routines in test.c. This allows running them
   in gdb, or adding debug prints. Prod code can use the DEBUGF macro
   to print messages in the self_test.

   The unit test is a work in progress, it does not yet support SMP or
   error injection. It does cover all the major test routines. Like
   memtest86+ proper, the unit test is a 32-bit binary. Unlike memtest86+
   the self test needs libc, so you will need a 32-bit libc installed.

 - Clean up the duplicative and brittle loop tests in test.c. There were
   address-dependent bugs here, eg. they would only manifest if a segment
   ended at exactly 4G or had a length that was a near multiple of SPINSZ.

   Some of these bugs were silent. For example test 7 would terminate
   early (without testing all memory) if the range from calculate_chunk()
   happens to map to the exact top of 4GB address space. It's possible
   that other tests suffered from the same bug. Others were louder;
   I fixed at least one infinite loop that was cropping up in test 3.

   Don't squish individual bugs; deny them a habitat. Use a shared
   'foreach_segment()' routine in all tests to set up the loops
   consistently for all tests. Unit test this routine to ensure it's
   right. Avoid overflows, not by being excruciatingly careful,
   but by computing loop bounds in terms of dword indices instead of
   byte addresses, so all quantities are far from the 4G overflow line.

 - Avoid 1- and 2-letter variable names. This is a work in progress.
   Favor names that are meaningful, or at least searchable.

 - Indent consistently. Replace tab characters with spaces so the code will
   render the same for everyone.

 - In test 7 (block moves) replace assembly with C for everything except
   the 'movsl' instruction that does all the real work of this test.
   The rewritten test performs within a measurement error of the original.
   I have a flaky machine which fails the original block_move test, and it
   fails the new one too with the same failure mode, as expected.

 - In test 10 (modtst) replace assembly with C. It had to be modified to
   work with the new loop bounds logic. I was less confident in my ability
   to modify assembly. GCC's output looks just as tight as the handwritten
   assembly.

   Side note: we're compiling test.c with -O2 now. Someone had knocked it
   down to -O0 in 5.01 (why? to make the assembly look better? to
   work around one of the many bugs that are now fixed?) In any event,
   things are working well at -O2 now. When I compared the compiler's code
   to the original assembly for modtst, it was -O2 output.

 - Rewrite test 1 (addr_tst1) to fix a bug. Before, it could possibly
   write to stray addresses that aren't mapped to DRAM (and could be
   mapped to hardware devices. Uh oh!) The simplified test combines
   the unbanked and banked modes, since the banked mode pretty much
   covered the unbanked mode. It uses the foreach_segment() routine
   and dword-indexed-based loop tests to avoid overflows.
