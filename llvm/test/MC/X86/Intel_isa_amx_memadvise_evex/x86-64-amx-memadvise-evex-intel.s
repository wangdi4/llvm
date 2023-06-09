// REQUIRES: intel_feature_isa_amx_memadvise_evex
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding  %s | FileCheck %s

// CHECK:      t2rpntlvwz0advisee tmm14, [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7c,0x08,0x76,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               t2rpntlvwz0advisee tmm14, [rbp + 8*r14 + 268435456], 123

// CHECK:      t2rpntlvwz0advisee tmm10, [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0x53,0x7c,0x08,0x76,0x94,0x80,0x23,0x01,0x00,0x00,0x7b]
               t2rpntlvwz0advisee tmm10, [r8 + 4*rax + 291], 123

// CHECK:      t2rpntlvwz0advisee tmm10, [2*rbp - 32], 123
// CHECK: encoding: [0x62,0x73,0x7c,0x08,0x76,0x14,0x6d,0xe0,0xff,0xff,0xff,0x7b]
               t2rpntlvwz0advisee tmm10, [2*rbp - 32], 123

// CHECK:      t2rpntlvwz1advisee tmm14, [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0x33,0x7d,0x08,0x76,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               t2rpntlvwz1advisee tmm14, [rbp + 8*r14 + 268435456], 123

// CHECK:      t2rpntlvwz1advisee tmm10, [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0x53,0x7d,0x08,0x76,0x94,0x80,0x23,0x01,0x00,0x00,0x7b]
               t2rpntlvwz1advisee tmm10, [r8 + 4*rax + 291], 123

// CHECK:      t2rpntlvwz1advisee tmm10, [2*rbp - 32], 123
// CHECK: encoding: [0x62,0x73,0x7d,0x08,0x76,0x14,0x6d,0xe0,0xff,0xff,0xff,0x7b]
               t2rpntlvwz1advisee tmm10, [2*rbp - 32], 123

