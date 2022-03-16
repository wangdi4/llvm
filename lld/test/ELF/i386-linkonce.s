// INTEL_CUSTOMIZATION
// Now this test case should not produce a reference to a discardable section
// since we have fixed the ".gnu.linkonce." issue.
// END INTEL_CUSTOMIZATION

// REQUIRES: x86
// RUN: llvm-mc -filetype=obj -triple=i386-linux-gnu %s -o %t.o
// RUN: llvm-mc -filetype=obj -triple=i386-linux-gnu %p/Inputs/i386-linkonce.s -o %t2.o
// RUN: llvm-ar rcs %t2.a %t2.o
// INTEL_CUSTOMIZATION
// BEGIN OLD CHECKS
// RUN-: not ld.lld %t.o %t2.a -o /dev/null 2>&1 | FileCheck %s

// CHECK-: error: relocation refers to a symbol in a discarded section: __i686.get_pc_thunk.bx
// END OLD CHECKS

// RUN: ld.lld %t.o %t2.a -o /dev/null 2>&1 | FileCheck --allow-empty %s
// CHECK-NOT: error: relocation refers to a symbol in a discarded section: __i686.get_pc_thunk.bx
// END INTEL_CUSTOMIZATION
# REQUIRES: x86
# RUN: rm -rf %t && split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=i386 %t/a.s -o %t/a.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/crti.s -o %t/crti.o
# RUN: llvm-mc -filetype=obj -triple=i386 %t/elf-init.s -o %t/elf-init.o

## crti.o in i386 glibc<2.32 has .gnu.linkonce.t.__x86.get_pc_thunk.bx that is
## not fully supported. Test that we don't report
## "relocation refers to a symbol in a discarded section: __x86.get_pc_thunk.bx".
# RUN: ld.lld %t/a.o %t/crti.o %t/elf-init.o -o /dev/null

#--- a.s
.globl _start
_start:

#--- crti.s
.section .gnu.linkonce.t.__x86.get_pc_thunk.bx,"ax"
.globl __x86.get_pc_thunk.bx
.hidden __x86.get_pc_thunk.bx
__x86.get_pc_thunk.bx:
  movl (%esp),%ebx
  ret

#--- elf-init.s
.globl __libc_csu_init
__libc_csu_init:
  call __x86.get_pc_thunk.bx

.section .text.__x86.get_pc_thunk.bx,"axG",@progbits,__x86.get_pc_thunk.bx,comdat
.globl __x86.get_pc_thunk.bx
.hidden __x86.get_pc_thunk.bx
__x86.get_pc_thunk.bx:
  movl (%esp),%ebx
  ret
