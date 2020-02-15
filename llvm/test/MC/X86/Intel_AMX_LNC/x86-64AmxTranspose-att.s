// REQUIRES: intel_feature_isa_amx_lnc
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s
// CHECK:      t2rpntlvw %r9, 268435456(%rbp,%r14,8), %tmm6
// CHECK: encoding: [0xc4,0xa5,0x30,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvw %r9, 268435456(%rbp,%r14,8), %tmm6

// CHECK:      t2rpntlvw %r9, 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc5,0x30,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvw %r9, 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvw %r9, -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe5,0x30,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvw %r9, -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwt1 %r9, 268435456(%rbp,%r14,8), %tmm6
// CHECK: encoding: [0xc4,0xa5,0x31,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwt1 %r9, 268435456(%rbp,%r14,8), %tmm6

// CHECK:      t2rpntlvwt1 %r9, 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc5,0x31,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwt1 %r9, 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwt1 %r9, -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe5,0x31,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwt1 %r9, -32(,%rbp,2), %tmm2
