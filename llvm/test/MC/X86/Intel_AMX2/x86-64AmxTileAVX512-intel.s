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

// When tile16move's 2nd operand is from zmm16 to zmm31, it will choose
// zmm16_zmm17_...zmm31 and use zmm16 represent it.
// CHECK:      tile16move tmm6, zmm16
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xf0]
               tile16move tmm6, zmm23

// CHECK:      tile16move tmm3, zmm16
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xd8]
               tile16move tmm3, zmm23

// CHECK:      tile16move tmm22, zmm16
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xf0]
               tile16move tmm22, zmm23

// CHECK:      tile16move tmm19, zmm16
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xd8]
               tile16move tmm19, zmm23
