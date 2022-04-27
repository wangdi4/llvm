// This test case replaces the community test case ELF/comdat-linkonce.s.
// It checks that the input files linked correctly since one of the symbols
// with name "abc" has a ".gnu.linkonce." section.
//
// NOTE: We don't need to use FileCheck because if the linker fails then it
// will exit with an error code and the lit test will fail.

// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %p/Inputs/comdat.s -o %t2.o
// RUN: ld.lld -shared %t.o %t2.o -o %t
// RUN: ld.lld -shared %t2.o %t.o -o %t

// Symbol abc has a ".gnu.linkonce" section.
.section .gnu.linkonce.t.abc
.globl abc
abc:
nop
