// REQUIRES: intel_feature_isa_amx_bf8
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tdpbf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps tmm6, tmm5, tmm4

// CHECK:      tdpbf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps tmm3, tmm2, tmm1

// CHECK:      tdpbf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps tmm6, tmm5, tmm4

// CHECK:      tdpbf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps tmm3, tmm2, tmm1

// CHECK:      tdpbf8ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps tmm6, tmm5, tmm4

// CHECK:      tdpbf8ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps tmm3, tmm2, tmm1
