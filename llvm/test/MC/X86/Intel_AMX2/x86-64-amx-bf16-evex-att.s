// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tdpbf16pse %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0x62,0xf2,0x5e,0x08,0x5c,0xf5]
               tdpbf16pse %tmm4, %tmm5, %tmm6

// CHECK:      tdpbf16pse %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0x62,0xf2,0x76,0x08,0x5c,0xda]
               tdpbf16pse %tmm1, %tmm2, %tmm3

// CHECK:      tdpbf16pse %tmm4, %tmm5, %tmm22
// CHECK: encoding: [0x62,0xe2,0x5e,0x08,0x5c,0xf5]
               tdpbf16pse %tmm4, %tmm5, %tmm22

// CHECK:      tdpbf16pse %tmm1, %tmm2, %tmm19
// CHECK: encoding: [0x62,0xe2,0x76,0x08,0x5c,0xda]
               tdpbf16pse %tmm1, %tmm2, %tmm19

// CHECK:      tdpbf16pse %tmm4, %tmm31, %tmm6
// CHECK: encoding: [0x62,0x92,0x5e,0x08,0x5c,0xf7]
               tdpbf16pse %tmm4, %tmm31, %tmm6

// CHECK:      tdpbf16pse %tmm21, %tmm30, %tmm23
// CHECK: encoding: [0x62,0x82,0x56,0x00,0x5c,0xfe]
               tdpbf16pse %tmm21, %tmm30, %tmm23


