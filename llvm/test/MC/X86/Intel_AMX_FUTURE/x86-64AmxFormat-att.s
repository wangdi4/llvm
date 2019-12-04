// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s
// CHECK:      tblendvd %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe6,0xf4]
               tblendvd %tmm4, %tmm5, %tmm6

// CHECK:      tblendvd %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe6,0xd9]
               tblendvd %tmm1, %tmm2, %tmm3

// CHECK:      tinterleaveeb %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x53,0xe8,0xf4]
               tinterleaveeb %tmm4, %tmm5, %tmm6

// CHECK:      tinterleaveeb %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6b,0xe8,0xd9]
               tinterleaveeb %tmm1, %tmm2, %tmm3

// CHECK:      tinterleaveew %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe8,0xf4]
               tinterleaveew %tmm4, %tmm5, %tmm6

// CHECK:      tinterleaveew %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe8,0xd9]
               tinterleaveew %tmm1, %tmm2, %tmm3

// CHECK:      tinterleaveob %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe8,0xf4]
               tinterleaveob %tmm4, %tmm5, %tmm6

// CHECK:      tinterleaveob %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe8,0xd9]
               tinterleaveob %tmm1, %tmm2, %tmm3

// CHECK:      tinterleaveow %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe8,0xf4]
               tinterleaveow %tmm4, %tmm5, %tmm6

// CHECK:      tinterleaveow %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe8,0xd9]
               tinterleaveow %tmm1, %tmm2, %tmm3

// CHECK:      tnarrowb $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x7a,0x58,0xf5,0x7b]
               tnarrowb $123, %tmm5, %tmm6

// CHECK:      tnarrowb $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x7a,0x58,0xda,0x7b]
               tnarrowb $123, %tmm2, %tmm3

// CHECK:      tnarroww $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x78,0x59,0xf5,0x7b]
               tnarroww $123, %tmm5, %tmm6

// CHECK:      tnarroww $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x78,0x59,0xda,0x7b]
               tnarroww $123, %tmm2, %tmm3

// CHECK:      tpermb %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe6,0xf4]
               tpermb %tmm4, %tmm5, %tmm6

// CHECK:      tpermb %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe6,0xd9]
               tpermb %tmm1, %tmm2, %tmm3

// CHECK:      tpermb 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x52,0xe6,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermb 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tpermb 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x6a,0xe6,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermb 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tpermb (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x52,0xe6,0x35,0x00,0x00,0x00,0x00]
               tpermb (%rip), %tmm5, %tmm6

// CHECK:      tpermb -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x6a,0xe6,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermb -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tpermd %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe7,0xf4]
               tpermd %tmm4, %tmm5, %tmm6

// CHECK:      tpermd %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe7,0xd9]
               tpermd %tmm1, %tmm2, %tmm3

// CHECK:      tpermd 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x51,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermd 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tpermd 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x69,0xe7,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermd 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tpermd (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x51,0xe7,0x35,0x00,0x00,0x00,0x00]
               tpermd (%rip), %tmm5, %tmm6

// CHECK:      tpermd -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x69,0xe7,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermd -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      tpermw %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe7,0xf4]
               tpermw %tmm4, %tmm5, %tmm6

// CHECK:      tpermw %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe7,0xd9]
               tpermw %tmm1, %tmm2, %tmm3

// CHECK:      tpermw 268435456(%rbp,%r14,8), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xa5,0x50,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tpermw 268435456(%rbp,%r14,8), %tmm5, %tmm6

// CHECK:      tpermw 291(%r8,%rax,4), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xc5,0x68,0xe7,0x9c,0x80,0x23,0x01,0x00,0x00]
               tpermw 291(%r8,%rax,4), %tmm2, %tmm3

// CHECK:      tpermw (%rip), %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x50,0xe7,0x35,0x00,0x00,0x00,0x00]
               tpermw (%rip), %tmm5, %tmm6

// CHECK:      tpermw -32(,%rbp,2), %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x68,0xe7,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tpermw -32(,%rbp,2), %tmm2, %tmm3

// CHECK:      twidenb $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x79,0x59,0xf5,0x7b]
               twidenb $123, %tmm5, %tmm6

// CHECK:      twidenb $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x79,0x59,0xda,0x7b]
               twidenb $123, %tmm2, %tmm3

// CHECK:      twidenw $123, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe3,0x7b,0x59,0xf5,0x7b]
               twidenw $123, %tmm5, %tmm6

// CHECK:      twidenw $123, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe3,0x7b,0x59,0xda,0x7b]
               twidenw $123, %tmm2, %tmm3
