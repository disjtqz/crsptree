
Requires at least C++2017. Tested with clang-cl, MSVC 2022, Intel ICC.

Started with a naive rbtree implementation from a book, decompiled the code that clang generated and then worked to reduce code size by replacing branches and identical code paths
with arithmetic that uses the offsets of the left and right members of the nodes. This step was repeated twice. Example: this implementation has no rotateNodeLeft or rotateNodeRight functions,
there is a rotate_by_offset method that does both.


small code size is emphasized, this is a headers only library but noinline is applied to most large critical functions with the expectation that the user
will have link-time optimization to take care of deduplicating the functions. This approach may lead to longer link times and larger debug builds, but it makes the implementation
of the functions available in all compilation units they are used in.

For an example of using the library you can look at tests/crsptree_test.cpp

In the future the interface will be changed a BIT to allow for specialized versions of these methods allowing for denser node structures via 32-bit pointers in 64 bit processes/32 bit based
pointers and to allow splitting up some operations into different smaller functions, allowing the user to schedule prefetches in between steps.