This directory contains binaries which are not typically directly invoked.
These binaries are not placed in PATH during environment setup in order to
reduce collisions with other toolchains.

This is a departure from OneAPI Compiler Releases before 2022.0 (December
2021), where these tools were in PATH. There may be cases where the tools which
are no longer in PATH were being invoked directly in some application Makefile
(or CMake configuration) and this may require adjustment:

1. If invoking "clang" or "clang++" directly, please try to use icx/icpx/dpcpp
   instead. Direct use of clang as a driver is not supported. The "clang"
   drivers do not have the same behavior as either icx or upstream LLVM, and
   are not widely tested.

2. If for some reason you must use the OneAPI clang/clang++, please report the
   issue. Until the issue is resolved, you can find the clang/clang++ driver's
   location by running "dpcpp --print-prog-name=clang".

3. If you use some other LLVM tool which is no longer in PATH, it can be found
   the same way. E.g.,: "dpcpp --print-prog-name=llvm-ar".

4. If all of the above fails, you can add the output of
   "dpcpp --print-file-name=bin-llvm" to PATH. This should be considered a last
   resort.

When switching from clang to icx/icpx/dpcpp, please be aware that the default
optimizations used by icx are much more aggressive. If this may be of concern,
you may specify the following flags to icx to very roughly approximate clang's
default optimizations:

- On Linux: -O0 -fno-fast-math
- On Windows: /O0 /fp:precise



