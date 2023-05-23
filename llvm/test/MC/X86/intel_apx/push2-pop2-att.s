# REQUIRES: intel_feature_isa_apx_f
# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s
# RUN: not llvm-mc -triple i386 -show-encoding %s 2>&1 | FileCheck %s --check-prefix=ERROR

# ERROR-COUNT-10: error:
# ERROR-NOT: error:
# CHECK: push2	$1, %rax, %rcx, $3
# CHECK: encoding: [0x62,0xf4,0x7c,0x1f,0xff,0xf1]
         push2	$1, %rax, %rcx, $3

# CHECK: push2	$1, %r16, %rcx, $3
# CHECK: encoding: [0x62,0xf4,0x7c,0x17,0xff,0xf1]
         push2	$1, %r16, %rcx, $3

# CHECK: push2	$1, %rax, %r17, $3
# CHECK: encoding: [0x62,0xfc,0x7c,0x1f,0xff,0xf1]
         push2	$1, %rax, %r17, $3

# CHECK: push2	$1, %r16, %r17, $3
# CHECK: encoding: [0x62,0xfc,0x7c,0x17,0xff,0xf1]
         push2	$1, %r16, %r17, $3

# CHECK: push2	$0, %rax, %rcx, $2
# CHECK: encoding: [0x62,0xf4,0x7c,0x1a,0xff,0xf1]
         push2	$0, %rax, %rcx, $2


# CHECK: pop2	$3, %rax, %rcx, $1
# CHECK: encoding: [0x62,0xf4,0x7c,0x1f,0x8f,0xc1]
         pop2	$3, %rax, %rcx, $1

# CHECK: pop2	$3, %r16, %rcx, $1
# CHECK: encoding: [0x62,0xf4,0x7c,0x17,0x8f,0xc1]
         pop2	$3, %r16, %rcx, $1

# CHECK: pop2	$3, %rax, %r17, $1
# CHECK: encoding: [0x62,0xfc,0x7c,0x1f,0x8f,0xc1]
         pop2	$3, %rax, %r17, $1

# CHECK: pop2	$3, %r16, %r17, $1
# CHECK: encoding: [0x62,0xfc,0x7c,0x17,0x8f,0xc1]
         pop2	$3, %r16, %r17, $1

# CHECK: pop2	$2, %rax, %rcx, $0
# CHECK: encoding: [0x62,0xf4,0x7c,0x1a,0x8f,0xc1]
         pop2	$2, %rax, %rcx, $0
