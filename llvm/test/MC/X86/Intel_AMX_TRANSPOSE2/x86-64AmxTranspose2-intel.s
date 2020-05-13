// REQUIRES: intel_feature_isa_amx_transpose2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      t2transposew tmm6, byte ptr [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x33,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2transposew tmm6, byte ptr [rbp + 8*r14 + 268435456], r9

// CHECK:      t2transposew tmm2, byte ptr [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x33,0xe7,0x94,0x80,0x23,0x01,0x00,0x00]
               t2transposew tmm2, byte ptr [r8 + 4*rax + 291], r9

// CHECK:      t2transposew tmm2, byte ptr [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x33,0xe7,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2transposew tmm2, byte ptr [2*rbp - 32], r9

// CHECK:      t2transposewt1 tmm6, byte ptr [rbp + 8*r14 + 268435456], r9
// CHECK: encoding: [0xc4,0xa5,0x32,0xe7,0xb4,0xf5,0x00,0x00,0x00,0x10]
               t2transposewt1 tmm6, byte ptr [rbp + 8*r14 + 268435456], r9

// CHECK:      t2transposewt1 tmm2, byte ptr [r8 + 4*rax + 291], r9
// CHECK: encoding: [0xc4,0xc5,0x32,0xe7,0x94,0x80,0x23,0x01,0x00,0x00]
               t2transposewt1 tmm2, byte ptr [r8 + 4*rax + 291], r9

// CHECK:      t2transposewt1 tmm2, byte ptr [2*rbp - 32], r9
// CHECK: encoding: [0xc4,0xe5,0x32,0xe7,0x14,0x6d,0xe0,0xff,0xff,0xff]
               t2transposewt1 tmm2, byte ptr [2*rbp - 32], r9

// CHECK:      t4rqntlvbz0 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x78,0xee,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz0 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz0 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x78,0xee,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz0 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz0 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x78,0xee,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz0 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz0t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x78,0xef,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz0t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz0t1 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x78,0xef,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz0t1 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz0t1 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x78,0xef,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz0t1 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz1 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x79,0xee,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz1 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz1 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x79,0xee,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz1 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz1 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x79,0xee,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz1 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz1t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x79,0xef,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz1t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz1t1 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x79,0xef,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz1t1 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz1t1 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x79,0xef,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz1t1 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz2 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7b,0xee,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz2 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz2 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7b,0xee,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz2 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz2 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x7b,0xee,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz2 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz2t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7b,0xef,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz2t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz2t1 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7b,0xef,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz2t1 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz2t1 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x7b,0xef,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz2t1 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz3 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7a,0xee,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz3 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz3 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7a,0xee,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz3 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz3 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x7a,0xee,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz3 tmm4, byte ptr [2*rbp - 32]

// CHECK:      t4rqntlvbz3t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa5,0x7a,0xef,0x84,0xf5,0x00,0x00,0x00,0x10]
               t4rqntlvbz3t1 tmm0, byte ptr [rbp + 8*r14 + 268435456]

// CHECK:      t4rqntlvbz3t1 tmm4, byte ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc5,0x7a,0xef,0xa4,0x80,0x23,0x01,0x00,0x00]
               t4rqntlvbz3t1 tmm4, byte ptr [r8 + 4*rax + 291]

// CHECK:      t4rqntlvbz3t1 tmm4, byte ptr [2*rbp - 32]
// CHECK: encoding: [0xc4,0xe5,0x7a,0xef,0x24,0x6d,0xe0,0xff,0xff,0xff]
               t4rqntlvbz3t1 tmm4, byte ptr [2*rbp - 32]
