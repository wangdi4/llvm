// REQUIRES: intel_feature_isa_avx_movget
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget xmm2, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x7a,0xc5,0x94,0xf5,0x00,0x00,0x00,0x10]
               vmovget xmm2, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vmovget xmm2, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x7a,0xc5,0x94,0x80,0x23,0x01,0x00,0x00]
               vmovget xmm2, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vmovget xmm2, xmmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x15,0x00,0x00,0x00,0x00]
               vmovget xmm2, xmmword ptr [rip]

// CHECK:      vmovget xmm2, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vmovget xmm2, xmmword ptr [2*rbp - 512]

// CHECK:      vmovget xmm2, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x91,0xf0,0x07,0x00,0x00]
               vmovget xmm2, xmmword ptr [rcx + 2032]

// CHECK:      vmovget xmm2, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x92,0x00,0xf8,0xff,0xff]
               vmovget xmm2, xmmword ptr [rdx - 2048]

// CHECK:      vmovget ymm2, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0xc4,0xa2,0x7e,0xc5,0x94,0xf5,0x00,0x00,0x00,0x10]
               vmovget ymm2, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vmovget ymm2, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0xc4,0xc2,0x7e,0xc5,0x94,0x80,0x23,0x01,0x00,0x00]
               vmovget ymm2, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vmovget ymm2, ymmword ptr [rip]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x15,0x00,0x00,0x00,0x00]
               vmovget ymm2, ymmword ptr [rip]

// CHECK:      vmovget ymm2, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vmovget ymm2, ymmword ptr [2*rbp - 1024]

// CHECK:      vmovget ymm2, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x91,0xe0,0x0f,0x00,0x00]
               vmovget ymm2, ymmword ptr [rcx + 4064]

// CHECK:      vmovget ymm2, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x92,0x00,0xf0,0xff,0xff]
               vmovget ymm2, ymmword ptr [rdx - 4096]

