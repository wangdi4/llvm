// REQUIRES: intel_feature_isa_amx_future
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s
// CHECK:      tcoladdbcastps %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe5,0x79,0xe6,0xf5]
               tcoladdbcastps %tmm5, %tmm6

// CHECK:      tcoladdbcastps %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe5,0x79,0xe6,0xda]
               tcoladdbcastps %tmm2, %tmm3

// CHECK:      tcoladdps %tmm6, 268435456(%rbp,%r14,8)
// CHECK: encoding: [0xc4,0xa5,0x78,0xe6,0xb4,0xf5,0x00,0x00,0x00,0x10]
               tcoladdps %tmm6, 268435456(%rbp,%r14,8)

// CHECK:      tcoladdps %tmm3, 291(%r8,%rax,4)
// CHECK: encoding: [0xc4,0xc5,0x78,0xe6,0x9c,0x80,0x23,0x01,0x00,0x00]
               tcoladdps %tmm3, 291(%r8,%rax,4)

// CHECK:      tcoladdps %tmm6, (%rip)
// CHECK: encoding: [0xc4,0xe5,0x78,0xe6,0x35,0x00,0x00,0x00,0x00]
               tcoladdps %tmm6, (%rip)

// CHECK:      tcoladdps %tmm3, -32(,%rbp,2)
// CHECK: encoding: [0xc4,0xe5,0x78,0xe6,0x1c,0x6d,0xe0,0xff,0xff,0xff]
               tcoladdps %tmm3, -32(,%rbp,2)
