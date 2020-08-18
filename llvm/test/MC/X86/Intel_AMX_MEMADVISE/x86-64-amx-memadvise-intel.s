// REQUIRES: intel_feature_isa_amx_memadvise
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding  %s | FileCheck %s

// CHECK:      tmovadvise tmm6, [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0xa3,0x7b,0x12,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               tmovadvise tmm6, [rbp + 8*r14 + 268435456], 123

// CHECK:      tmovadvise tmm3, [r8 + 4*rax + 291], 123
// CHECK: encoding: [0xc4,0xc3,0x7b,0x12,0x9c,0x80,0x23,0x01,0x00,0x00,0x7b]
               tmovadvise tmm3, [r8 + 4*rax + 291], 123

// CHECK:      tmovadvise tmm3, [2*rbp - 32], 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x12,0x1c,0x6d,0xe0,0xff,0xff,0xff,0x7b]
               tmovadvise tmm3, [2*rbp - 32], 123

// CHECK:      tmovadvise [rcx + 127], tmm6, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x13,0x74,0x21,0x7f,0x7b]
               tmovadvise [rcx + 127], tmm6, 123

// CHECK:      tmovadvise [rdx - 128], tmm3, 123
// CHECK: encoding: [0xc4,0xe3,0x7b,0x13,0x5c,0x22,0x80,0x7b]
               tmovadvise [rdx - 128], tmm3, 123

// CHECK:      tmovadvise [rbp + 8*r14 + 268435456], tmm6, 123
// CHECK: encoding: [0xc4,0xa3,0x7b,0x13,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               tmovadvise [rbp + 8*r14 + 268435456], tmm6, 123

// CHECK:      tmovadvise [r8 + 4*rax + 291], tmm3, 123
// CHECK: encoding: [0xc4,0xc3,0x7b,0x13,0x9c,0x80,0x23,0x01,0x00,0x00,0x7b]
               tmovadvise [r8 + 4*rax + 291], tmm3, 123
