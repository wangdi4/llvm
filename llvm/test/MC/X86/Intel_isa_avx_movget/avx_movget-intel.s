// REQUIRES: intel_feature_isa_avx_movget
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               vmovget xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vmovget xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               vmovget xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vmovget xmm2, xmmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x10]
               vmovget xmm2, xmmword ptr [eax]

// CHECK:      vmovget xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x14,0x6d,0x00,0xfe,0xff,0xff]
               vmovget xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      vmovget xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x91,0xf0,0x07,0x00,0x00]
               vmovget xmm2, xmmword ptr [ecx + 2032]

// CHECK:      vmovget xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0xc4,0xe2,0x7a,0xc5,0x92,0x00,0xf8,0xff,0xff]
               vmovget xmm2, xmmword ptr [edx - 2048]

// CHECK:      vmovget ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               vmovget ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vmovget ymm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               vmovget ymm2, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vmovget ymm2, ymmword ptr [eax]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x10]
               vmovget ymm2, ymmword ptr [eax]

// CHECK:      vmovget ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x14,0x6d,0x00,0xfc,0xff,0xff]
               vmovget ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      vmovget ymm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x91,0xe0,0x0f,0x00,0x00]
               vmovget ymm2, ymmword ptr [ecx + 4064]

// CHECK:      vmovget ymm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0xc4,0xe2,0x7e,0xc5,0x92,0x00,0xf0,0xff,0xff]
               vmovget ymm2, ymmword ptr [edx - 4096]

