# Alignment check on windows.

# RUN: llvm-mc -filetype=obj -triple x86_64-windows %s -o %t1
# RUN: lld-link %t1 -out:%t2 /subsystem:console -entry:_start
# RUN: llvm-objdump --traceback %t1 | FileCheck %s --check-prefixes=OBJ
# RUN: llvm-objdump --traceback %t2 | FileCheck %s --check-prefixes=BINARY

# OBJ: Expect 8-byte align for section .trace!
## BINARY-NOT: Expect 8-byte align for section .trace!

.global _start
.text
_start:
    nop

.section    .trace,"dr"
.byte 0
