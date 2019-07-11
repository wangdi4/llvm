// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tilemov2zmm     zmm2, tmm1, 128
// CHECK: encoding: [0x62,0xf3,0x7d,0x48,0x07,0xd1,0x80]
               tilemov2zmm     zmm2, tmm1, 128
