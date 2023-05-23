# REQUIRES: intel_feature_isa_apx_f
# RUN: llvm-mc -triple x86_64 -show-encoding %s | FileCheck %s
# RUN: not llvm-mc -triple i386 -show-encoding %s 2>&1 | FileCheck %s --check-prefix=ERROR

# ERROR-COUNT-27: error:
# ERROR-NOT: error:
## Condition flags

# CHECK: ccmpoq {}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0x84,0x10,0x39,0xc3]
         ccmpoq {}	%rax, %rbx
# CHECK: ccmpoq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x10,0x39,0xc3]
         ccmpoq {of}	%rax, %rbx
# CHECK: ccmpoq {sf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xa4,0x10,0x39,0xc3]
         ccmpoq {sf}	%rax, %rbx
# CHECK: ccmpoq {zf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0x94,0x10,0x39,0xc3]
         ccmpoq {zf}	%rax, %rbx
# CHECK: ccmpoq {cf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0x8c,0x10,0x39,0xc3]
         ccmpoq {cf}	%rax, %rbx
# CHECK: ccmpoq {of,sf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xe4,0x10,0x39,0xc3]
         ccmpoq {of,sf}	%rax, %rbx
# CHECK: ccmpoq {of,sf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xe4,0x10,0x39,0xc3]
         ccmpoq {sf,of}	%rax, %rbx
# CHECK: ccmpoq {of,sf,zf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xf4,0x10,0x39,0xc3]
         ccmpoq {of,sf,zf}	%rax, %rbx
# CHECK: ccmpoq {of,sf,zf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xf4,0x10,0x39,0xc3]
         ccmpoq {zf,of,sf}	%rax, %rbx
# CHECK: ccmpoq {of,sf,zf,cf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xfc,0x10,0x39,0xc3]
         ccmpoq {of,sf,zf,cf}	%rax, %rbx
# CHECK: ccmpoq {of,sf,zf,cf}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xfc,0x10,0x39,0xc3]
         ccmpoq {cf,zf,sf,of}	%rax, %rbx

## Condition code

# CHECK: ccmpnoq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x11,0x39,0xc3]
         ccmpnoq {of}	%rax, %rbx
# CHECK: ccmpbq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x12,0x39,0xc3]
         ccmpbq {of}	%rax, %rbx
# CHECK: ccmpaeq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x13,0x39,0xc3]
         ccmpaeq {of}	%rax, %rbx
# CHECK: ccmpeq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x14,0x39,0xc3]
         ccmpeq {of}	%rax, %rbx
# CHECK: ccmpneq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x15,0x39,0xc3]
         ccmpneq {of}	%rax, %rbx
# CHECK: ccmpbeq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x16,0x39,0xc3]
         ccmpbeq {of}	%rax, %rbx
# CHECK: ccmpaq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x17,0x39,0xc3]
         ccmpaq {of}	%rax, %rbx
# CHECK: ccmpsq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x18,0x39,0xc3]
         ccmpsq {of}	%rax, %rbx
# CHECK: ccmpnsq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x19,0x39,0xc3]
         ccmpnsq {of}	%rax, %rbx
# CHECK: ccmppq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1a,0x39,0xc3]
         ccmppq {of}	%rax, %rbx
# CHECK: ccmpnpq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1b,0x39,0xc3]
         ccmpnpq {of}	%rax, %rbx
# CHECK: ccmplq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1c,0x39,0xc3]
         ccmplq {of}	%rax, %rbx
# CHECK: ccmpgeq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1d,0x39,0xc3]
         ccmpgeq {of}	%rax, %rbx
# CHECK: ccmpleq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1e,0x39,0xc3]
         ccmpleq {of}	%rax, %rbx
# CHECK: ccmpgq {of}	%rax, %rbx
# CHECK: encoding: [0x62,0xf4,0xc4,0x1f,0x39,0xc3]
         ccmpgq {of}	%rax, %rbx

## 32/16/8-bit

# CHECK: ccmpbl {sf}	%eax, %ebx
# CHECK: encoding: [0x62,0xf4,0x24,0x12,0x39,0xc3]
         ccmpbl {sf}	%eax, %ebx
