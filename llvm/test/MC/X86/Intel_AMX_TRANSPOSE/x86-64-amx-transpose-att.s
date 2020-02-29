// REQUIRES: intel_feature_isa_amx_transpose
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      ttdpbf16ps %tmm8, %tmm10, %tmm15
// CHECK: encoding: [0xc4,0x42,0x3a,0x6c,0xfa]
               ttdpbf16ps %tmm8, %tmm10, %tmm15

// CHECK:      ttdpbf16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x72,0x6c,0xda]
               ttdpbf16ps %tmm1, %tmm2, %tmm3

// CHECK:      ttdpfp16ps %tmm8, %tmm10, %tmm15
// CHECK: encoding: [0xc4,0x42,0x3b,0x6c,0xfa]
               ttdpfp16ps %tmm8, %tmm10, %tmm15

// CHECK:      ttdpfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x73,0x6c,0xda]
               ttdpfp16ps %tmm1, %tmm2, %tmm3

// CHECK:      ttransposed %tmm10, %tmm15
// CHECK: encoding: [0xc4,0x42,0x7a,0x5f,0xfa]
               ttransposed %tmm10, %tmm15

// CHECK:      ttransposed %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x7a,0x5f,0xda]
               ttransposed %tmm2, %tmm3

// CHECK:      t2rpntlvw %r9, 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x25,0x30,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvw %r9, 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvw %r9, 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc5,0x30,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvw %r9, 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvw %r9, -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe5,0x30,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvw %r9, -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwt1 %r9, 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x25,0x31,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwt1 %r9, 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvwt1 %r9, 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc5,0x31,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwt1 %r9, 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwt1 %r9, -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe5,0x31,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwt1 %r9, -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwz0 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x22,0x78,0x6e,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz0 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvwz0 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc2,0x78,0x6e,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz0 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwz0 -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x6e,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz0 -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwz0t1 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x22,0x78,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz0t1 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvwz0t1 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc2,0x78,0x6f,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz0t1 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwz0t1 -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x78,0x6f,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz0t1 -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwz1 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x22,0x79,0x6e,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz1 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvwz1 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc2,0x79,0x6e,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz1 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwz1 -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x6e,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz1 -32(,%rbp,2), %tmm2

// CHECK:      t2rpntlvwz1t1 268435456(%rbp,%r14,8), %tmm14
// CHECK: encoding: [0xc4,0x22,0x79,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz1t1 268435456(%rbp,%r14,8), %tmm14

// CHECK:      t2rpntlvwz1t1 291(%r8,%rax,4), %tmm2
// CHECK: encoding: [0xc4,0xc2,0x79,0x6f,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz1t1 291(%r8,%rax,4), %tmm2

// CHECK:      t2rpntlvwz1t1 -32(,%rbp,2), %tmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x6f,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz1t1 -32(,%rbp,2), %tmm2
