// REQUIRES: intel_feature_isa_amx_lnc
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s
// CHECK:      t2rpntlvw tmm6, [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x30,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvw tmm6, [rbp + 8*r14 + 268435456], r9

// CHECK:      t2rpntlvw tmm2, [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x30,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvw tmm2, [r8 + 4*rax + 291], r9

// CHECK:      t2rpntlvw tmm2, [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x30,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvw tmm2, [2*rbp - 32], r9

// CHECK:      t2rpntlvwt1 tmm6, [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x31,0xe9,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2rpntlvwt1 tmm6, [rbp + 8*r14 + 268435456], r9

// CHECK:      t2rpntlvwt1 tmm2, [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x31,0xe9,0x94,0x80,0x23,0x01,0x00,0x00]
               t2rpntlvwt1 tmm2, [r8 + 4*rax + 291], r9

// CHECK:      t2rpntlvwt1 tmm2, [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x31,0xe9,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2rpntlvwt1 tmm2, [2*rbp - 32], r9
