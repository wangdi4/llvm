# Check the dump tool doesn't crash and output diagnostic information if it find illegal format or byte.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s -o %t1
# RUN: ld.lld  %t1 -o %t2
# RUN: llvm-objdump --traceback %t1 | FileCheck %s
# RUN: llvm-objdump --traceback %t2 | FileCheck %s

# CHECK: Expect 8-byte align for section .trace!
# CHECK-NEXT: .trace contents:
# CHECK-NEXT: 00000000: ff                       # Unknown byte

.global _start
.text
_start:
    nop

.section    .trace,"a",@progbits
.byte 255
