// Check that the compilation graph is not required.
// RUN: tblgen -I %p/../../include --gen-llvmc %s -o %t
// RUN: %compile_cxx %t
// XFAIL: vg_leak

; XFAIL: *
include "llvm/CompilerDriver/Common.td"
