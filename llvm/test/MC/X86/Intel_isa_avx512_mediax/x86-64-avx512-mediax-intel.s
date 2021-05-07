// REQUIRES: intel_feature_isa_avx512_mediax
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      vmpsadbw zmm22, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x40,0x42,0xf0,0x7b]
               vmpsadbw zmm22, zmm23, zmm24, 123

// CHECK:      vmpsadbw zmm22 {k7}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0x47,0x42,0xf0,0x7b]
               vmpsadbw zmm22 {k7}, zmm23, zmm24, 123

// CHECK:      vmpsadbw zmm22 {k7} {z}, zmm23, zmm24, 123
// CHECK: encoding: [0x62,0x83,0x46,0xc7,0x42,0xf0,0x7b]
               vmpsadbw zmm22 {k7} {z}, zmm23, zmm24, 123

// CHECK:      vmpsadbw zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123
// CHECK: encoding: [0x62,0xa3,0x46,0x40,0x42,0xb4,0xf5,0x00,0x00,0x00,0x10,0x7b]
               vmpsadbw zmm22, zmm23, zmmword ptr [rbp + 8*r14 + 268435456], 123

// CHECK:      vmpsadbw zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123
// CHECK: encoding: [0x62,0xc3,0x46,0x47,0x42,0xb4,0x80,0x23,0x01,0x00,0x00,0x7b]
               vmpsadbw zmm22 {k7}, zmm23, zmmword ptr [r8 + 4*rax + 291], 123

// CHECK:      vmpsadbw zmm22, zmm23, zmmword ptr [rip], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x40,0x42,0x35,0x00,0x00,0x00,0x00,0x7b]
               vmpsadbw zmm22, zmm23, zmmword ptr [rip], 123

// CHECK:      vmpsadbw zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123
// CHECK: encoding: [0x62,0xe3,0x46,0x40,0x42,0x34,0x6d,0x00,0xf8,0xff,0xff,0x7b]
               vmpsadbw zmm22, zmm23, zmmword ptr [2*rbp - 2048], 123

// CHECK:      vmpsadbw zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123
// CHECK: encoding: [0x62,0xe3,0x46,0xc7,0x42,0x71,0x7f,0x7b]
               vmpsadbw zmm22 {k7} {z}, zmm23, zmmword ptr [rcx + 8128], 123

// CHECK:      vmpsadbw zmm22 {k7} {z}, zmm23, zmmword ptr [rdx - 8192], 123
// CHECK: encoding: [0x62,0xe3,0x46,0xc7,0x42,0x72,0x80,0x7b]
               vmpsadbw zmm22 {k7} {z}, zmm23, zmmword ptr [rdx - 8192], 123
