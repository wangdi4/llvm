// REQUIRES: intel_feature_isa_sm3
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vsm3msg1 xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x60,0xda,0xd4]
               vsm3msg1 xmm2, xmm3, xmm4

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x60,0xda,0x94,0xf5,0x00,0x00,0x00,0x10]
               vsm3msg1 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x60,0xda,0x94,0x80,0x23,0x01,0x00,0x00]
               vsm3msg1 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x60,0xda,0x15,0x00,0x00,0x00,0x00]
               vsm3msg1 xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x60,0xda,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vsm3msg1 xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x60,0xda,0x91,0xf0,0x07,0x00,0x00]
               vsm3msg1 xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vsm3msg1 xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x60,0xda,0x92,0x00,0xf8,0xff,0xff]
               vsm3msg1 xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vsm3msg2 xmm2, xmm3, xmm4
// CHECK: encoding: [0xc4,0xe2,0x61,0xda,0xd4]
               vsm3msg2 xmm2, xmm3, xmm4

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x61,0xda,0x94,0xf5,0x00,0x00,0x00,0x10]
               vsm3msg2 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x61,0xda,0x94,0x80,0x23,0x01,0x00,0x00]
               vsm3msg2 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x61,0xda,0x15,0x00,0x00,0x00,0x00]
               vsm3msg2 xmm2, xmm3, xmmword ptr [rip]

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x61,0xda,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vsm3msg2 xmm2, xmm3, xmmword ptr [2*rbp - 512]

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x61,0xda,0x91,0xf0,0x07,0x00,0x00]
               vsm3msg2 xmm2, xmm3, xmmword ptr [rcx + 2032]

// CHECK:      vsm3msg2 xmm2, xmm3, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x61,0xda,0x92,0x00,0xf8,0xff,0xff]
               vsm3msg2 xmm2, xmm3, xmmword ptr [rdx - 2048]

// CHECK:      vsm3rnds2 xmm2, xmm3, xmm4, 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xde,0xd4,0x7b]
               vsm3rnds2 xmm2, xmm3, xmm4, 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa3,0x61,0xde,0x94,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc3,0x61,0xde,0x94,0x80,0x23,0x01,0x00,0x00,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [rip], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xde,0x15,0x00,0x00,0x00,0x00,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [rip], 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [2*rbp - 512], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xde,0x14,0x6d,0x00,0xfe,0xff,0xff,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [2*rbp - 512], 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xde,0x91,0xf0,0x07,0x00,0x00,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [rcx + 2032], 123

// CHECK:      vsm3rnds2 xmm2, xmm3, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0xe3,0x61,0xde,0x92,0x00,0xf8,0xff,0xff,0x7b]
               vsm3rnds2 xmm2, xmm3, xmmword ptr [rdx - 2048], 123

