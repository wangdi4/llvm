// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget xmm2, xmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vmovget xmm2, xmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vmovget xmm2, xmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vmovget xmm2, xmmword ptr [edi + 4*eax + 291]

// CHECK:      vmovget xmm2, xmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x10]
               {evex} vmovget xmm2, xmmword ptr [eax]

// CHECK:      vmovget xmm2, xmmword ptr [2*ebp - 512]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x14,0x6d,0x00,0xfe,0xff,0xff]
               {evex} vmovget xmm2, xmmword ptr [2*ebp - 512]

// CHECK:      vmovget xmm2, xmmword ptr [ecx + 2032]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x51,0x7f]
               {evex} vmovget xmm2, xmmword ptr [ecx + 2032]

// CHECK:      vmovget xmm2, xmmword ptr [edx - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x08,0xc5,0x52,0x80]
               {evex} vmovget xmm2, xmmword ptr [edx - 2048]

// CHECK:      vmovget ymm2, ymmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               {evex} vmovget ymm2, ymmword ptr [esp + 8*esi + 268435456]

// CHECK:      vmovget ymm2, ymmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               {evex} vmovget ymm2, ymmword ptr [edi + 4*eax + 291]

// CHECK:      vmovget ymm2, ymmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x10]
               {evex} vmovget ymm2, ymmword ptr [eax]

// CHECK:      vmovget ymm2, ymmword ptr [2*ebp - 1024]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x14,0x6d,0x00,0xfc,0xff,0xff]
               {evex} vmovget ymm2, ymmword ptr [2*ebp - 1024]

// CHECK:      vmovget ymm2, ymmword ptr [ecx + 4064]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x51,0x7f]
               {evex} vmovget ymm2, ymmword ptr [ecx + 4064]

// CHECK:      vmovget ymm2, ymmword ptr [edx - 4096]
// CHECK: encoding: [0x62,0xf2,0x7e,0x28,0xc5,0x52,0x80]
               {evex} vmovget ymm2, ymmword ptr [edx - 4096]

