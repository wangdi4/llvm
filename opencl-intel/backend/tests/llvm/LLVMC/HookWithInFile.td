// Check that a hook can be given $INFILE as an argument.
// RUN: tblgen -I %p/../../include --gen-llvmc %s -o %t
// RUN: FileCheck -input-file %t %s
// RUN: %compile_cxx %t
// XFAIL: vg_leak

; XFAIL: *
include "llvm/CompilerDriver/Common.td"

def dummy_tool : Tool<[
// CHECK: Hook(inFile.c_str())
(command "$CALL(Hook, '$INFILE')/path"),
(in_language "dummy"),
(out_language "dummy")
]>;

def DummyGraph : CompilationGraph<[(edge "root", "dummy_tool")]>;
