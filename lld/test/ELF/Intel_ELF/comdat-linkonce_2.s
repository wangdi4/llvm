// This test case checks that the linker fails with duplicate symbols message
// since neither symbol with name "abc" has a ".gnu.linkonce" section.

// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=x86_64-pc-linux %p/Inputs/comdat.s -o %t2.o
// RUN: not ld.lld -shared %t.o %t2.o -o %t 2>&1 | FileCheck %s
// RUN: not ld.lld -shared %t2.o %t.o -o %t 2>&1 | FileCheck %s

// CHECK: ld.lld: error: duplicate symbol: abc

.globl abc
abc:
nop
