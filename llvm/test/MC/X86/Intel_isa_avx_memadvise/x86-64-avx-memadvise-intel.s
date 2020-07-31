// REQUIRES: intel_feature_isa_avx_memadvise
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x7a,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmm12, xmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew xmm12, xmmword ptr [r9], 123
// CHECK: encoding: [0xc4,0x43,0x7a,0x10,0x21,0x7b]
               vmovadvisew xmm12, xmmword ptr [r9], 123

// CHECK:      vmovadvisew xmm12, xmmword ptr [rcx + 2032], 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x10,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               vmovadvisew xmm12, xmmword ptr [rcx + 2032], 123

// CHECK:      vmovadvisew xmm12, xmmword ptr [rdx - 2048], 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x10,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               vmovadvisew xmm12, xmmword ptr [rdx - 2048], 123

// CHECK:      vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0xc4,0x23,0x7e,0x10,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymm12, ymmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmovadvisew ymm12, ymmword ptr [r9], 123
// CHECK: encoding: [0xc4,0x43,0x7e,0x10,0x21,0x7b]
               vmovadvisew ymm12, ymmword ptr [r9], 123

// CHECK:      vmovadvisew ymm12, ymmword ptr [rcx + 4064], 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x10,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               vmovadvisew ymm12, ymmword ptr [rcx + 4064], 123

// CHECK:      vmovadvisew ymm12, ymmword ptr [rdx - 4096], 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x10,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               vmovadvisew ymm12, ymmword ptr [rdx - 4096], 123

// CHECK:      vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm12, 123
// CHECK: encoding: [0xc4,0x23,0x7a,0x11,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew xmmword ptr [rbp + 8*r14 + 268435456], xmm12, 123

// CHECK:      vmovadvisew xmmword ptr [r9], xmm12, 123
// CHECK: encoding: [0xc4,0x43,0x7a,0x11,0x21,0x7b]
               vmovadvisew xmmword ptr [r9], xmm12, 123

// CHECK:      vmovadvisew xmmword ptr [rcx + 2032], xmm12, 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x11,0xa1,0xf0,0x07,0x00,0x00,0x7b]
               vmovadvisew xmmword ptr [rcx + 2032], xmm12, 123

// CHECK:      vmovadvisew xmmword ptr [rdx - 2048], xmm12, 123
// CHECK: encoding: [0xc4,0x63,0x7a,0x11,0xa2,0x00,0xf8,0xff,0xff,0x7b]
               vmovadvisew xmmword ptr [rdx - 2048], xmm12, 123

// CHECK:      vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm12, 123
// CHECK: encoding: [0xc4,0x23,0x7e,0x11,0xa4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmovadvisew ymmword ptr [rbp + 8*r14 + 268435456], ymm12, 123

// CHECK:      vmovadvisew ymmword ptr [r9], ymm12, 123
// CHECK: encoding: [0xc4,0x43,0x7e,0x11,0x21,0x7b]
               vmovadvisew ymmword ptr [r9], ymm12, 123

// CHECK:      vmovadvisew ymmword ptr [rcx + 4064], ymm12, 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x11,0xa1,0xe0,0x0f,0x00,0x00,0x7b]
               vmovadvisew ymmword ptr [rcx + 4064], ymm12, 123

// CHECK:      vmovadvisew ymmword ptr [rdx - 4096], ymm12, 123
// CHECK: encoding: [0xc4,0x63,0x7e,0x11,0xa2,0x00,0xf0,0xff,0xff,0x7b]
               vmovadvisew ymmword ptr [rdx - 4096], ymm12, 123
