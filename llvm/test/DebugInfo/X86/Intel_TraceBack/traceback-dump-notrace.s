# Check the dump tool doesn't crash when it can not find .trace section and report an error.

# RUN: llvm-mc -filetype=obj -triple x86_64 %s | llvm-objdump --traceback - | FileCheck %s

# CHECK: Can not find section .trace

.text
.byte 0
