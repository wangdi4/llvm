// REQUIRES: intel_feature_isa_amx_bf8
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tdpbf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps %tmm4, %tmm5, %tmm6

// CHECK:      tdpbf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps %tmm1, %tmm2, %tmm3

// CHECK:      tdpbf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps %tmm4, %tmm5, %tmm6

// CHECK:      tdpbf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps %tmm1, %tmm2, %tmm3

// CHECK:      tdpbf8ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x59,0x5c,0xf5]
               tdpbf8ps %tmm4, %tmm5, %tmm6

// CHECK:      tdpbf8ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x71,0x5c,0xda]
               tdpbf8ps %tmm1, %tmm2, %tmm3
