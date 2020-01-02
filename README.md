# memtest

Fork of memtest86+ 5.01. Improves correctness, code health, and code readability
while maintaining the performance and coverage at levels at least as good as 5.01.

Non-goals of this fork:
 - Support UEFI
 - Build as a 64-bit binary
 - Improve SMP support. It's still experimental as always. I have done most
   testing in single core mode.


## TODO

 - unit test foreach_segment

 - apply foreach_segment to more tests

 - cover all test routines in self_test

 - call self_test from the Makefile

 - DEBUGF needs a stub printf definition in the prod binary

 - Apply the rest of the ubuntu/debian patches.
   Get in touch with maintainers...

 - Test SMP a little bit, ensure it's not borked. Getting SMP fully
   stable is not a goal but let's not make it worse.

 - Update the version number to 6.0. (Where does the 5.01 print from?
   I can't find the string in the source...)

## Changes since 5.01

 - Specify units on major APIs, data structures, and magic-number
   macros. Use comments and naming to clarify whether a given
   integer represents a physical address, virtual address,
   dword index, cache line index, page number, etc.

 - Add an ASSERT() macro for testing assumptions and catching coding
   bugs. Prints a message in yellow (not red) since assertion failures
   are not usually memory errors.

 - Add a usermode self_test. This is not a memory test, it's a unit test
   for the core memtest86+ routines in test.c, so we can run them in
   a debugger. We can also add debug prints to prod code with the DEBUGF
   macro.

   The unit test is a work in progress, it does not yet support SMP or
   error injection. It does cover all the major test routines. Like
   memtest86+ itself, the unit test is a 32-bit binary. Since it links
   with libc, you will need a 32-bit libc.

 - Clean up the duplicative and brittle loop tests in test.c. There were
   latent bugs here. They were address-dependent, eg they would only manifest
   if a segment ended at exactly 4G or had a length that was a near multiple
   of SPINSZ.

   Some of these bugs were silent. For example test 7 would terminate
   early (without testing all memory) if the range from calculate_chunk()
   happens to map to the exact top of 4GB address space. It's possible
   that other tests suffered from the same bug.

   Don't just squish the bugs; deny them a habitat. Replace the inline,
   duplicated loop tests with a shared 'foreach_segment()' routine
   that most tests can use. This routine:
    * is unit-tested for the edge-cases that broke the original loop tests.
    * asserts that chunks returned by calculate_chunk() are aligned
      as tests expect.
    * avoids overflows, not by being excruciatingly careful, but by
      computing the loop bounds in terms of dword indices (ranging up
      to 1G) instead of byte addresses (ranging up to the 4G overflow
      boundary.)

 - Avoid single-letter variable names. This is a work in progress.
   Favor names that are meaningful, or at least searchable.

 - Indent consistently. Replace tab characters with spaces so the code will
   render the same for everyone.

 - In test 7 (block moves) replace assembly with C for everything except
   the 'movsl' instruction that does the heavy lifting. The rewritten test
   performs within a measurement error of the original, and should be easier
   to inspect and maintain. It still fails on a flaky machine which
   also fails the original block move test, with the same failure mode.

 - In test 10 (modtst) replace assembly with C. It had to be modified to
   work with the new loop bounds logic. I would rather maintain C than
   assembly. I confirmed that gcc's output looks just as tight
   as the original handwritten assembly.
