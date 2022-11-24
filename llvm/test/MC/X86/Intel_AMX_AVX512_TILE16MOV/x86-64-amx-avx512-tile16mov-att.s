// REQUIRES: intel_feature_isa_amx_avx512_tile16mov
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
