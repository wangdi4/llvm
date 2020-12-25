// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget xmm22, xmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x08,0xc5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vmovget xmm22, xmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vmovget xmm22, xmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x08,0xc5,0xb4,0x80,0x23,0x01,0x00,0x00]
               vmovget xmm22, xmmword ptr [r8 + 4*rax + 291]

// CHECK:      vmovget xmm22, xmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xc5,0x35,0x00,0x00,0x00,0x00]
               vmovget xmm22, xmmword ptr [rip]

// CHECK:      vmovget xmm22, xmmword ptr [2*rbp - 512]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xc5,0x34,0x6d,0x00,0xfe,0xff,0xff]
               vmovget xmm22, xmmword ptr [2*rbp - 512]

// CHECK:      vmovget xmm22, xmmword ptr [rcx + 2032]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xc5,0x71,0x7f]
               vmovget xmm22, xmmword ptr [rcx + 2032]

// CHECK:      vmovget xmm22, xmmword ptr [rdx - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x08,0xc5,0x72,0x80]
               vmovget xmm22, xmmword ptr [rdx - 2048]

// CHECK:      vmovget ymm22, ymmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x28,0xc5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vmovget ymm22, ymmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vmovget ymm22, ymmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x28,0xc5,0xb4,0x80,0x23,0x01,0x00,0x00]
               vmovget ymm22, ymmword ptr [r8 + 4*rax + 291]

// CHECK:      vmovget ymm22, ymmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xc5,0x35,0x00,0x00,0x00,0x00]
               vmovget ymm22, ymmword ptr [rip]

// CHECK:      vmovget ymm22, ymmword ptr [2*rbp - 1024]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xc5,0x34,0x6d,0x00,0xfc,0xff,0xff]
               vmovget ymm22, ymmword ptr [2*rbp - 1024]

// CHECK:      vmovget ymm22, ymmword ptr [rcx + 4064]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xc5,0x71,0x7f]
               vmovget ymm22, ymmword ptr [rcx + 4064]

// CHECK:      vmovget ymm22, ymmword ptr [rdx - 4096]
// CHECK: encoding: [0x62,0xe2,0x7e,0x28,0xc5,0x72,0x80]
               vmovget ymm22, ymmword ptr [rdx - 4096]

