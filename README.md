# memtest

Fork of memtest86+ 5.01 with improvements on correctness and code readability.

## TODO

 - Fix test 2. why does it hang? maybe track down ubuntu's sources?
   get a clean end to end pass.

 - Update the version number. Where does the 5.01 print from?
   Consider whether to go to version 6 or be cute and go to
   version 64 or something...

## Changes since 5.01

 - Add an ASSERT() macro for testing assumptions and catching coding
   bugs. The message prints in yellow, not red, since assertion failures
   are not usually memory errors.

 - Specify units on major APIs, data structures, and magic-number
   macros. Use comments and naming to clarify whether a given
   integer represents a physical address, virtual address,
   dword index, cache line index, page number, etc.

 - Avoid single-letter variable names. This is a work in progress.
   Favor names that are meaningful, or at least searchable.

 - Indent consistently. Replace tab characters with spaces so the code will
   render the same for everyone.

 - Rewrite test 7 (block moves) with several improvements:
   - Assert that the results of calculate_chunk() are aligned sanely
     and consistently with our assumptions.
   - Avoid duplicating the loop tests.
   - Simplify the loop tests. Instead of being excruciatingly careful,
     we can avoid overflows by computing loop bounds in terms of dword
     indices (ranging up to 1G) instead of raw pointers (ranging up to
     4G and the overflow boundary.)
   - Fix a bug that would cause the test to terminate prematurely when
     the range from calculate_chunk() happens to map to the exact top of the
     4GB address space.
   - Replace assembly with C for everything except the 'movsl' instruction
     that does the heavy lifting. The rewritten test performs within a
     measurement error of the original, and should be easier to inspect
     and maintain.
