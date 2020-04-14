// REQUIRES: intel_feature_isa_amx_tile2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tilemov tmm6, tmm5
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0xf5]
               tilemov tmm6, tmm5

// CHECK:      tilemov tmm3, tmm2
// CHECK: encoding: [0xc4,0xe2,0x7b,0x4b,0xda]
               tilemov tmm3, tmm2

