// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple i686-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget zmm2, zmmword ptr [esp + 8*esi + 268435456]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x94,0xf4,0x00,0x00,0x00,0x10]
               vmovget zmm2, zmmword ptr [esp + 8*esi + 268435456]

// CHECK:      vmovget zmm2, zmmword ptr [edi + 4*eax + 291]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x94,0x87,0x23,0x01,0x00,0x00]
               vmovget zmm2, zmmword ptr [edi + 4*eax + 291]

// CHECK:      vmovget zmm2, zmmword ptr [eax]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x10]
               vmovget zmm2, zmmword ptr [eax]

// CHECK:      vmovget zmm2, zmmword ptr [2*ebp - 2048]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x14,0x6d,0x00,0xf8,0xff,0xff]
               vmovget zmm2, zmmword ptr [2*ebp - 2048]

// CHECK:      vmovget zmm2, zmmword ptr [ecx + 8128]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x51,0x7f]
               vmovget zmm2, zmmword ptr [ecx + 8128]

// CHECK:      vmovget zmm2, zmmword ptr [edx - 8192]
// CHECK: encoding: [0x62,0xf2,0x7e,0x48,0xc5,0x52,0x80]
               vmovget zmm2, zmmword ptr [edx - 8192]

