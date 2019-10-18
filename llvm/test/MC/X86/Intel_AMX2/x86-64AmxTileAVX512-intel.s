// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tilemovrowe zmm22, tmm14, xmm24
// CHECK: encoding: [0x62,0xc2,0x3c,0x40,0x4a,0xf6]
               tilemovrowe zmm22, tmm14, xmm24

// CHECK:      tilemovrowe zmm22, tmm30, xmm24
// CHECK: encoding: [0x62,0x82,0x3c,0x40,0x4a,0xf6]
               tilemovrowe zmm22, tmm30, xmm24

// CHECK:      tilemovrowe zmm22, tmm14, edx
// CHECK: encoding: [0x62,0xc2,0x6d,0x48,0x4a,0xf6]
               tilemovrowe zmm22, tmm14, edx

// CHECK:      tilemovrowe zmm22, tmm30, edx
// CHECK: encoding: [0x62,0x82,0x6d,0x48,0x4a,0xf6]
               tilemovrowe zmm22, tmm30, edx

// CHECK:      tilemovrowe zmm22, tmm14, 123
// CHECK: encoding: [0x62,0xc3,0x7d,0x48,0x07,0xf6,0x7b]
               tilemovrowe zmm22, tmm14, 123

// CHECK:      tilemovrowe zmm22, tmm30, 123
// CHECK: encoding: [0x62,0x83,0x7d,0x48,0x07,0xf6,0x7b]
               tilemovrowe zmm22, tmm30, 123

// CHECK:      tile16move tmm6, zmm23
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xf7]
               tile16move tmm6, zmm23

// CHECK:      tile16move tmm3, zmm23
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xdf]
               tile16move tmm3, zmm23

// CHECK:      tile16move tmm22, zmm23
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xf7]
               tile16move tmm22, zmm23

// CHECK:      tile16move tmm19, zmm23
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xdf]
               tile16move tmm19, zmm23
