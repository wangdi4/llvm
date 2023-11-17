# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s
# RUN: not llvm-mc -triple i386 -show-encoding %s 2>&1 | FileCheck %s --check-prefix=ERROR

# ERROR-COUNT-8: error:
# ERROR-NOT: error:
# CHECK: push2	%rax, %rcx
# CHECK: encoding: [0x62,0xf4,0x7c,0x18,0xff,0xf1]
         push2	%rax, %rcx
# CHECK: push2	%r16, %rcx
# CHECK: encoding: [0x62,0xf4,0x7c,0x10,0xff,0xf1]
         push2	%r16, %rcx
# CHECK: push2	%rax, %r17
# CHECK: encoding: [0x62,0xfc,0x7c,0x18,0xff,0xf1]
         push2	%rax, %r17
# CHECK: push2	%r16, %r17
# CHECK: encoding: [0x62,0xfc,0x7c,0x10,0xff,0xf1]
         push2	%r16, %r17

# CHECK: pop2	%rax, %rcx
# CHECK: encoding: [0x62,0xf4,0x7c,0x18,0x8f,0xc1]
         pop2	%rax, %rcx
# CHECK: pop2	%r16, %rcx
# CHECK: encoding: [0x62,0xf4,0x7c,0x10,0x8f,0xc1]
         pop2	%r16, %rcx
# CHECK: pop2	%rax, %r17
# CHECK: encoding: [0x62,0xfc,0x7c,0x18,0x8f,0xc1]
         pop2	%rax, %r17
# CHECK: pop2	%r16, %r17
# CHECK: encoding: [0x62,0xfc,0x7c,0x10,0x8f,0xc1]
         pop2	%r16, %r17
