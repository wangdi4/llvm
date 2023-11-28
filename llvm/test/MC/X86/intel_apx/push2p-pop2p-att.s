# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s
# RUN: not llvm-mc -triple i386 -show-encoding %s 2>&1 | FileCheck %s --check-prefix=ERROR

# ERROR-COUNT-8: error:
# ERROR-NOT: error:

# CHECK: push2p	%rax, %rcx
# CHECK: encoding: [0x62,0xf4,0xfc,0x18,0xff,0xf1]
         push2p	%rax, %rcx
# CHECK: push2p	%r16, %rcx
# CHECK: encoding: [0x62,0xf4,0xfc,0x10,0xff,0xf1]
         push2p	%r16, %rcx
# CHECK: push2p	%rax, %r17
# CHECK: encoding: [0x62,0xfc,0xfc,0x18,0xff,0xf1]
         push2p	%rax, %r17
# CHECK: push2p	%r16, %r17
# CHECK: encoding: [0x62,0xfc,0xfc,0x10,0xff,0xf1]
         push2p	%r16, %r17
# CHECK: pop2p	%rax, %rcx
# CHECK: encoding: [0x62,0xf4,0xfc,0x18,0x8f,0xc1]
         pop2p	%rax, %rcx
# CHECK: pop2p	%r16, %rcx
# CHECK: encoding: [0x62,0xf4,0xfc,0x10,0x8f,0xc1]
         pop2p	%r16, %rcx
# CHECK: pop2p	%rax, %r17
# CHECK: encoding: [0x62,0xfc,0xfc,0x18,0x8f,0xc1]
         pop2p	%rax, %r17
# CHECK: pop2p	%r16, %r17
# CHECK: encoding: [0x62,0xfc,0xfc,0x10,0x8f,0xc1]
         pop2p	%r16, %r17
