// REQUIRES: intel_feature_isa_amx_transpose
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:     ttdpbf16ps     tmm5, tmm0, tmm4
// CHECK: encoding: [0xc4,0xe2,0x5a,0x6c,0xe8]
               ttdpbf16ps tmm5, tmm0, tmm4

// CHECK:     ttdpbf16ps     tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x72,0x6c,0xda]
               ttdpbf16ps tmm3, tmm2, tmm1

// CHECK:     ttdpfp16ps     tmm1, tmm0, tmm4
// CHECK: encoding: [0xc4,0xe2,0x5b,0x6c,0xc8]
               ttdpfp16ps tmm1, tmm0, tmm4

// CHECK:     ttdpfp16ps     tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x73,0x6c,0xda]
               ttdpfp16ps tmm3, tmm2, tmm1

// CHECK:     ttransposed     tmm5, tmm1
// CHECK: encoding: [0xc4,0xe2,0x7a,0x5f,0xe9]
               ttransposed tmm5, tmm1

// CHECK:     ttransposed     tmm3, tmm2
// CHECK: encoding: [0xc4,0xe2,0x7a,0x5f,0xda]
               ttransposed tmm3, tmm2

// CHECK:     t2rpntlvw     tmm4, [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x30,0xe9,0xa4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvw tmm4, [rbp + 8*r14 + 268435456], r9

// CHECK:     t2rpntlvw     tmm2, [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x30,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvw tmm2, [r8 + 4*rax + 291], r9

// CHECK:     t2rpntlvw     tmm2, [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x30,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvw tmm2, [2*rbp - 32], r9

// CHECK:     t2rpntlvwt1     tmm4, [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x31,0xe9,0xa4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwt1 tmm5, [rbp + 8*r14 + 268435456], r9

// CHECK:     t2rpntlvwt1     tmm2, [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x31,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwt1 tmm2, [r8 + 4*rax + 291], r9

// CHECK:     t2rpntlvwt1     tmm2, [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x31,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwt1 tmm2, [2*rbp - 32], r9

// CHECK:     t2rpntlvwz0     tmm6, [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x78,0x6e,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz0 tmm6, [rbp + 8*r14 + 268435456]

// CHECK:     t2rpntlvwz0     tmm2, [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x78,0x6e,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz0 tmm2, [r8 + 4*rax + 291]

// CHECK:     t2rpntlvwz0     tmm2, [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe2,0x78,0x6e,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz0 tmm2, [2*rbp - 32]

// CHECK:     t2rpntlvwz0t1     tmm6, [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x78,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz0t1 tmm7, [rbp + 8*r14 + 268435456]

// CHECK:     t2rpntlvwz0t1     tmm2, [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x78,0x6f,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz0t1 tmm2, [r8 + 4*rax + 291]

// CHECK:     t2rpntlvwz0t1     tmm2, [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe2,0x78,0x6f,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz0t1 tmm2, [2*rbp - 32]

// CHECK:     t2rpntlvwz1     tmm0, [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x79,0x6e,0x84,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz1 tmm1, [rbp + 8*r14 + 268435456]

// CHECK:     t2rpntlvwz1     tmm2, [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x79,0x6e,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz1 tmm2, [r8 + 4*rax + 291]

// CHECK:     t2rpntlvwz1     tmm2, [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe2,0x79,0x6e,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz1 tmm2, [2*rbp - 32]

// CHECK:     t2rpntlvwz1t1     tmm6, [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x79,0x6f,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwz1t1 tmm6, [rbp + 8*r14 + 268435456]

// CHECK:     t2rpntlvwz1t1     tmm2, [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x79,0x6f,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwz1t1 tmm2, [r8 + 4*rax + 291]

// CHECK:     t2rpntlvwz1t1     tmm2, [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe2,0x79,0x6f,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwz1t1 tmm2, [2*rbp - 32]
