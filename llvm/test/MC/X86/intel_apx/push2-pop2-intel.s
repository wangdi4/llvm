# REQUIRES: intel_feature_isa_apx_f
# RUN: llvm-mc -triple x86_64 -show-encoding -x86-asm-syntax=intel -output-asm-variant=1 %s | FileCheck %s

# CHECK: push2	3, rcx, rax, 1
# CHECK: encoding: [0x62,0xf4,0x7c,0x1f,0xff,0xf1]
         push2	3, rcx, rax, 1

# CHECK: push2	3, rcx, r16, 1
# CHECK: encoding: [0x62,0xf4,0x7c,0x17,0xff,0xf1]
         push2	3, rcx, r16, 1

# CHECK: push2	3, r17, rax, 1
# CHECK: encoding: [0x62,0xfc,0x7c,0x1f,0xff,0xf1]
         push2	3, r17, rax, 1

# CHECK: push2	3, r17, r16, 1
# CHECK: encoding: [0x62,0xfc,0x7c,0x17,0xff,0xf1]
         push2	3, r17, r16, 1

# CHECK: push2	2, rcx, rax, 0
# CHECK: encoding: [0x62,0xf4,0x7c,0x1a,0xff,0xf1]
         push2	2, rcx, rax, 0


# CHECK: pop2	1, rcx, rax, 3
# CHECK: encoding: [0x62,0xf4,0x7c,0x1f,0x8f,0xc1]
         pop2	1, rcx, rax, 3

# CHECK: pop2	1, rcx, r16, 3
# CHECK: encoding: [0x62,0xf4,0x7c,0x17,0x8f,0xc1]
         pop2	1, rcx, r16, 3

# CHECK: pop2	1, r17, rax, 3
# CHECK: encoding: [0x62,0xfc,0x7c,0x1f,0x8f,0xc1]
         pop2	1, r17, rax, 3

# CHECK: pop2	1, r17, r16, 3
# CHECK: encoding: [0x62,0xfc,0x7c,0x17,0x8f,0xc1]
         pop2	1, r17, r16, 3

# CHECK: pop2	0, rcx, rax, 2
# CHECK: encoding: [0x62,0xf4,0x7c,0x1a,0x8f,0xc1]
         pop2	0, rcx, rax, 2
