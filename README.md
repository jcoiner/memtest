# memtest

Fork of memtest86+ 5.01 with improvements on correctness and code readability.

## Changes since 5.01

 - Add an ASSERT() macro that we can use to detect coding bugs
   and bad assumptions. Asserts don't generally indicate a memory error,
   that's not their intent. Thus the message prints in yellow, not red
   like traditional memory errors.

 - Specify units on major APIs, data structures, and magic-number
   macros. Use comments and naming to clarify whether a given
   integer represents a physical address, virtual address,
   cache line index, page index, etc.

 - Avoid single-letter variable names. This is a work in progress.
   Favor names that are meaningful, or at least searchable.

 - Indent consistently. Replace tab characters with spaces so the code will
   render the same for everyone.

 - Rewrite test 7 (block moves) with several improvements:
   - Avoid duplicating the loop tests.
   - Simplify the loop tests. Instead of being excruciatingly careful,
     avoid overflows by computing loop bounds in terms of dword indices
     (ranging up to 1G) instead of raw pointers (ranging up to
     4G and the overflow boundary.)
   - Assert that the results of calculate_chunk() are aligned sanely
     and consistently with our assumptions.
   - Fix a bug that would cause the test to terminate prematurely when
     the range from calculate_chunk() happens to map to the exact top of the
     4GB address space (which can happen.)
   - Replace assembly with C for everything except the critical
     'movsl' instruction that does the heavy lifting. The rewritten
     test performs within measurement error of the original, and
     should be easier to inspect and maintain.
