// REQUIRES: intel_feature_isa_amx_transpose
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tile16move      %zmm0, %tmm6
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x5f,0xf0]
               tile16move      %zmm0, %tmm6

// CHECK:      tile16move      %zmm0, %tmm3
// CHECK: encoding: [0x62,0xf2,0x7d,0x48,0x5f,0xd8]
               tile16move      %zmm0, %tmm3

// CHECK:      tile16move      %zmm0, %tmm22
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x5f,0xf0]
               tile16move      %zmm0, %tmm22

// CHECK:      tile16move      %zmm0, %tmm19
// CHECK: encoding: [0x62,0xe2,0x7d,0x48,0x5f,0xd8]
               tile16move      %zmm0, %tmm19

// CHECK:      tile16move      %zmm16, %tmm6
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xf0]
               tile16move      %zmm16, %tmm6

// CHECK:      tile16move      %zmm16, %tmm3
// CHECK: encoding: [0x62,0xb2,0x7d,0x48,0x5f,0xd8]
               tile16move      %zmm16, %tmm3

// CHECK:      tile16move      %zmm16, %tmm22
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xf0]
               tile16move      %zmm16, %tmm22

// CHECK:      tile16move      %zmm16, %tmm19
// CHECK: encoding: [0x62,0xa2,0x7d,0x48,0x5f,0xd8]
               tile16move      %zmm16, %tmm19

// CHECK:      tcvtrowd2ps %edx, %tmm14, %zmm22
// CHECK: encoding: [0x62,0xc2,0x6e,0x48,0x4a,0xf6]
               tcvtrowd2ps %edx, %tmm14, %zmm22

// CHECK:      tcvtrowd2ps %edx, %tmm30, %zmm22
// CHECK: encoding: [0x62,0x82,0x6e,0x48,0x4a,0xf6]
               tcvtrowd2ps %edx, %tmm30, %zmm22

// CHECK:      tcvtrowd2ps $123, %tmm14, %zmm22
// CHECK: encoding: [0x62,0xc3,0x7e,0x48,0x07,0xf6,0x7b]
               tcvtrowd2ps $123, %tmm14, %zmm22

// CHECK:      tcvtrowd2ps $123, %tmm30, %zmm22
// CHECK: encoding: [0x62,0x83,0x7e,0x48,0x07,0xf6,0x7b]
               tcvtrowd2ps $123, %tmm30, %zmm22
