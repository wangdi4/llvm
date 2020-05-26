// REQUIRES: intel_feature_isa_amx_tile2
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tilemov %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0xf5]
               tilemov %tmm5, %tmm6

// CHECK:      tilemov %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0xda]
               tilemov %tmm2, %tmm3

