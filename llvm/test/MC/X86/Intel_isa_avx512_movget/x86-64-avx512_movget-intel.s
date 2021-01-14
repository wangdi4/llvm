// REQUIRES: intel_feature_isa_avx512_movget
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmovget zmm22, zmmword ptr [rbp + 8*r14 + 268435456]
// CHECK: encoding: [0x62,0xa2,0x7e,0x48,0xc5,0xb4,0xf5,0x00,0x00,0x00,0x10]
               vmovget zmm22, zmmword ptr [rbp + 8*r14 + 268435456]

// CHECK:      vmovget zmm22, zmmword ptr [r8 + 4*rax + 291]
// CHECK: encoding: [0x62,0xc2,0x7e,0x48,0xc5,0xb4,0x80,0x23,0x01,0x00,0x00]
               vmovget zmm22, zmmword ptr [r8 + 4*rax + 291]

// CHECK:      vmovget zmm22, zmmword ptr [rip]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xc5,0x35,0x00,0x00,0x00,0x00]
               vmovget zmm22, zmmword ptr [rip]

// CHECK:      vmovget zmm22, zmmword ptr [2*rbp - 2048]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xc5,0x34,0x6d,0x00,0xf8,0xff,0xff]
               vmovget zmm22, zmmword ptr [2*rbp - 2048]

// CHECK:      vmovget zmm22, zmmword ptr [rcx + 8128]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xc5,0x71,0x7f]
               vmovget zmm22, zmmword ptr [rcx + 8128]

// CHECK:      vmovget zmm22, zmmword ptr [rdx - 8192]
// CHECK: encoding: [0x62,0xe2,0x7e,0x48,0xc5,0x72,0x80]
               vmovget zmm22, zmmword ptr [rdx - 8192]

