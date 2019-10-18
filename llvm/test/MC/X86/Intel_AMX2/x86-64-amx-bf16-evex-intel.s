// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tdpbf16pse tmm6, tmm5, tmm4
// CHECK: encoding: [0x62,0xf2,0x5e,0x08,0x5c,0xf5]
               tdpbf16pse tmm6, tmm5, tmm4

// CHECK:      tdpbf16pse tmm3, tmm2, tmm1
// CHECK: encoding: [0x62,0xf2,0x76,0x08,0x5c,0xda]
               tdpbf16pse tmm3, tmm2, tmm1

// CHECK:      tdpbf16pse tmm22, tmm5, tmm4
// CHECK: encoding: [0x62,0xe2,0x5e,0x08,0x5c,0xf5]
               tdpbf16pse tmm22, tmm5, tmm4

// CHECK:      tdpbf16pse tmm19, tmm2, tmm1
// CHECK: encoding: [0x62,0xe2,0x76,0x08,0x5c,0xda]
               tdpbf16pse tmm19, tmm2, tmm1

// CHECK:      tdpbf16pse tmm6, tmm31, tmm4
// CHECK: encoding: [0x62,0x92,0x5e,0x08,0x5c,0xf7]
               tdpbf16pse tmm6, tmm31, tmm4

// CHECK:      tdpbf16pse tmm23, tmm30, tmm21
// CHECK: encoding: [0x62,0x82,0x56,0x00,0x5c,0xfe]
               tdpbf16pse tmm23, tmm30, tmm21


