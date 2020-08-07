# Check the dump tool doesn't crash and output diagnostic information if it find illegal format or byte.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK: Expect 8-byte align for section .trace!
# CHECK-NEXT: .trace contents:
# CHECK-NEXT: 00000000: ff                       # Unknown byte

.section    .trace,"a",@progbits
.byte 255
