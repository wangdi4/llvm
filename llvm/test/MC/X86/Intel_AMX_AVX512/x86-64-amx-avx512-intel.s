// REQUIRES: intel_feature_isa_amx_avx512
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tcvtrowd2pse zmm22, tmm19, ecx
// CHECK: encoding: [0x62,0xa2,0x76,0x48,0x4a,0xf3]
               tcvtrowd2pse zmm22, tmm19, ecx

// CHECK:      tcvtrowd2pse zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x4a,0xf2]
               tcvtrowd2pse zmm22, tmm2, ecx

// CHECK:      tcvtrowd2pse zmm22, tmm19, 123
// CHECK: encoding: [0x62,0xa3,0x7e,0x48,0x07,0xf3,0x7b]
               tcvtrowd2pse zmm22, tmm19, 123

// CHECK:      tcvtrowd2pse zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x07,0xf2,0x7b]
               tcvtrowd2pse zmm22, tmm2, 123

// CHECK:      tcvtrowps2bf16e zmm22, tmm19, ecx
// CHECK: encoding: [0x62,0xa2,0x77,0x48,0x6d,0xf3]
               tcvtrowps2bf16e zmm22, tmm19, ecx

// CHECK:      tcvtrowps2bf16e zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x77,0x48,0x6d,0xf2]
               tcvtrowps2bf16e zmm22, tmm2, ecx

// CHECK:      tcvtrowps2bf16e zmm22, tmm19, 123
// CHECK: encoding: [0x62,0xa3,0x7f,0x48,0x07,0xf3,0x7b]
               tcvtrowps2bf16e zmm22, tmm19, 123

// CHECK:      tcvtrowps2bf16e zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x07,0xf2,0x7b]
               tcvtrowps2bf16e zmm22, tmm2, 123

// CHECK:      tcvtrowps2phe zmm22, tmm19, ecx
// CHECK: encoding: [0x62,0xa2,0x74,0x48,0x6d,0xf3]
               tcvtrowps2phe zmm22, tmm19, ecx

// CHECK:      tcvtrowps2phe zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x74,0x48,0x6d,0xf2]
               tcvtrowps2phe zmm22, tmm2, ecx

// CHECK:      tcvtrowps2phe zmm22, tmm19, 123
// CHECK: encoding: [0x62,0xa3,0x7c,0x48,0x07,0xf3,0x7b]
               tcvtrowps2phe zmm22, tmm19, 123

// CHECK:      tcvtrowps2phe zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x07,0xf2,0x7b]
               tcvtrowps2phe zmm22, tmm2, 123

// CHECK:      tile16move tmm31, zmm16
// CHECK: encoding: [0x62,0x22,0x7d,0x48,0x5f,0xf8]
               tile16move tmm31, zmm16

// CHECK:      tile16move tmm15, zmm16
// CHECK: encoding: [0x62,0x32,0x7d,0x48,0x5f,0xf8]
               tile16move tmm15, zmm16

// CHECK:      tilemovrowe zmm22, tmm19, xmm24
// CHECK: encoding: [0x62,0xa2,0x3c,0x40,0x4a,0xf3]
               tilemovrowe zmm22, tmm19, xmm24

// CHECK:      tilemovrowe zmm22, tmm2, xmm24
// CHECK: encoding: [0x62,0xe2,0x3c,0x40,0x4a,0xf2]
               tilemovrowe zmm22, tmm2, xmm24

// CHECK:      tilemovrowe zmm22, tmm19, ecx
// CHECK: encoding: [0x62,0xa2,0x75,0x48,0x4a,0xf3]
               tilemovrowe zmm22, tmm19, ecx

// CHECK:      tilemovrowe zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf2]
               tilemovrowe zmm22, tmm2, ecx

// CHECK:      tilemovrowe zmm22, tmm19, 123
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x07,0xf3,0x7b]
               tilemovrowe zmm22, tmm19, 123

// CHECK:      tilemovrowe zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf2,0x7b]
               tilemovrowe zmm22, tmm2, 123

