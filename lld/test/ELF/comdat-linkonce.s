// INTEL_CUSTOMIZATION
// UNSUPPORTED: *
// This test case is marked as unsupported since we fixed the issue with
// gnu.linkonce section. The test case was built based on a hack made by
// the community (https://reviews.llvm.org/D28430 and
// https://reviews.llvm.org/D47748). They discard any section with the names
// ".gnu.linkonce.t.__x86.get_pc_thunk.bx" and
// ".gnu.linkonce.t.__i686.get_pc_thunk.bx". This is wrong because the names
// of the ".gnu.linkonce." sections are ".gnu.linkonce.T.SYMBOLNAME", where
// T is the type and SYMBOLNAME is the symbol's name. The linker uses the
// symbols' names to catch if there is a ".gnu.linkonce" section and make a
// decision on which symbol resolution it should use. There are some special
// cases where .T is not present in the name, for example
// ".gnu.linkonce.this_module".
//
// This test case is replaced with the test Intel_ELF/comdat-linkonce.s.
// END INTEL_CUSTOMIZATION
// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %p/Inputs/comdat.s -o %t2.o
// RUN: ld.lld -shared %t.o %t2.o -o %t
// RUN: ld.lld -shared %t2.o %t.o -o %t

.section .gnu.linkonce.t.__x86.get_pc_thunk.bx
.globl abc
abc:
nop

.section .gnu.linkonce.t.__i686.get_pc_thunk.bx
.globl def
def:
nop
