# memtest

Branch of memtest86+ 5.01. Improves its correctness, code health, and code
readability while doing no harm to performance and coverage.

## TODO

 - Attempt to update reloc.c from a recent glibc that knows how
   to relocate more ELF section types. I suspect we aren't completely
   relocating everything we need to; see comment on the STATIC
   macro in test.c.

## Changes since 5.01

 - Specify units on major APIs, data structures, and shared constants.

 - Add an ASSERT() macro for validating assumptions. Prints a message
   in yellow to distinguish asserts from memory errors (printed in red.)

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

 - Rewrite test 1 (addr\_tst1) to fix a bug. Before, it could possibly
   write to stray addresses that aren't mapped to DRAM -- and could be
   mapped to hardware devices, uh oh! The simplified test combines
   the unbanked and banked modes, since the banked mode essentially
   covered the unbanked mode. It uses the foreach_segment() routine
   and dword-indexed-based loop tests to avoid overflows.
