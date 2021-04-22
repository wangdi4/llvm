// REQUIRES: intel_feature_isa_amx_avx512
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tile16move %zmm16, %tmm31
// CHECK: encoding: [0x62,0x22,0x7d,0x48,0x5f,0xf8]
               tile16move %zmm16, %tmm31

// CHECK:      tile16move %zmm16, %tmm15
// CHECK: encoding: [0x62,0x32,0x7d,0x48,0x5f,0xf8]
               tile16move %zmm16, %tmm15

// CHECK:      tile16move %zmm0, %tmm6
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x5f,0xf0]
               tile16move %zmm0, %tmm6

// CHECK:      tile16move %zmm0, %tmm3
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x5f,0xd8]
               tile16move %zmm0, %tmm3

// CHECK:      tile16move %zmm0, %tmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x5f,0xf0]
               tile16move %zmm0, %tmm22

// CHECK:      tile16move %zmm0, %tmm19
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x5f,0xd8]
               tile16move %zmm0, %tmm19

// CHECK:      tile16move %zmm16, %tmm6
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xf0]
               tile16move %zmm16, %tmm6

// CHECK:      tile16move %zmm16, %tmm3
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xd8]
               tile16move %zmm16, %tmm3

// CHECK:      tile16move %zmm16, %tmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xf0]
               tile16move %zmm16, %tmm22

// CHECK:      tile16move %zmm16, %tmm19
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xd8]
               tile16move %zmm16, %tmm19

// CHECK:      tilemovrowe %xmm24, %tmm19, %zmm22
// CHECK: encoding: [0x62,0xa2,0x3c,0x40,0x4a,0xf3]
               tilemovrowe %xmm24, %tmm19, %zmm22

// CHECK:      tilemovrowe %xmm24, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x3c,0x40,0x4a,0xf2]
               tilemovrowe %xmm24, %tmm2, %zmm22

// CHECK:      tilemovrowe %ecx, %tmm19, %zmm22
// CHECK: encoding: [0x62,0xa2,0x75,0x48,0x4a,0xf3]
               tilemovrowe %ecx, %tmm19, %zmm22

// CHECK:      tilemovrowe %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x4a,0xf2]
               tilemovrowe %ecx, %tmm2, %zmm22

// CHECK:      tilemovrowe $123, %tmm19, %zmm22
// CHECK: encoding: [0x62,0xa3,0x7d,0x48,0x07,0xf3,0x7b]
               tilemovrowe $123, %tmm19, %zmm22

// CHECK:      tilemovrowe $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7d,0x48,0x07,0xf2,0x7b]
               tilemovrowe $123, %tmm2, %zmm22

// CHECK:      tilemovrowe %xmm24, %tmm14, %zmm22
// CHECK: encoding: [0x62,0xc2,0x3c,0x40,0x4a,0xf6]
               tilemovrowe %xmm24, %tmm14, %zmm22

// CHECK:      tilemovrowe %edx, %tmm14, %zmm22
// CHECK: encoding: [0x62,0xc2,0x6d,0x48,0x4a,0xf6]
               tilemovrowe %edx, %tmm14, %zmm22

// CHECK:      tilemovrowe $123, %tmm14, %zmm22
// CHECK: encoding: [0x62,0xc3,0x7d,0x48,0x07,0xf6,0x7b]
               tilemovrowe $123, %tmm14, %zmm22
