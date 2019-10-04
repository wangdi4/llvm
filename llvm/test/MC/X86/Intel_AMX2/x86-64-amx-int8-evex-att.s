// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding %s | FileCheck %s

// CHECK:      tdpbssde %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0x62,0xf2,0x5f,0x08,0x5e,0xf5]
               tdpbssde %tmm4, %tmm5, %tmm6

// CHECK:      tdpbssde %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0x62,0xf2,0x77,0x08,0x5e,0xda]
               tdpbssde %tmm1, %tmm2, %tmm3

// CHECK:      tdpbsude %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0x62,0xf2,0x5e,0x08,0x5e,0xf5]
               tdpbsude %tmm4, %tmm5, %tmm6

// CHECK:      tdpbsude %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0x62,0xf2,0x76,0x08,0x5e,0xda]
               tdpbsude %tmm1, %tmm2, %tmm3

// CHECK:      tdpbusde %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0x62,0xf2,0x5d,0x08,0x5e,0xf5]
               tdpbusde %tmm4, %tmm5, %tmm6

// CHECK:      tdpbusde %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0x62,0xf2,0x75,0x08,0x5e,0xda]
               tdpbusde %tmm1, %tmm2, %tmm3

// CHECK:      tdpbuude %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0x62,0xf2,0x5c,0x08,0x5e,0xf5]
               tdpbuude %tmm4, %tmm5, %tmm6

// CHECK:      tdpbuude %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0x62,0xf2,0x74,0x08,0x5e,0xda]
               tdpbuude %tmm1, %tmm2, %tmm3

// CHECK:      tdpbssde %tmm4, %tmm5, %tmm22
// CHECK: encoding: [0x62,0xe2,0x5f,0x08,0x5e,0xf5]
               tdpbssde %tmm4, %tmm5, %tmm22

// CHECK:      tdpbssde %tmm1, %tmm2, %tmm19
// CHECK: encoding: [0x62,0xe2,0x77,0x08,0x5e,0xda]
               tdpbssde %tmm1, %tmm2, %tmm19

// CHECK:      tdpbsude %tmm4, %tmm5, %tmm22
// CHECK: encoding: [0x62,0xe2,0x5e,0x08,0x5e,0xf5]
               tdpbsude %tmm4, %tmm5, %tmm22

// CHECK:      tdpbsude %tmm1, %tmm2, %tmm19
// CHECK: encoding: [0x62,0xe2,0x76,0x08,0x5e,0xda]
               tdpbsude %tmm1, %tmm2, %tmm19

// CHECK:      tdpbusde %tmm4, %tmm5, %tmm22
// CHECK: encoding: [0x62,0xe2,0x5d,0x08,0x5e,0xf5]
               tdpbusde %tmm4, %tmm5, %tmm22

// CHECK:      tdpbusde %tmm1, %tmm2, %tmm19
// CHECK: encoding: [0x62,0xe2,0x75,0x08,0x5e,0xda]
               tdpbusde %tmm1, %tmm2, %tmm19

// CHECK:      tdpbuude %tmm4, %tmm5, %tmm22
// CHECK: encoding: [0x62,0xe2,0x5c,0x08,0x5e,0xf5]
               tdpbuude %tmm4, %tmm5, %tmm22

// CHECK:      tdpbuude %tmm1, %tmm2, %tmm19
// CHECK: encoding: [0x62,0xe2,0x74,0x08,0x5e,0xda]
               tdpbuude %tmm1, %tmm2, %tmm19

// CHECK:      tdpbssde %tmm4, %tmm31, %tmm6
// CHECK: encoding: [0x62,0x92,0x5f,0x08,0x5e,0xf7]
               tdpbssde %tmm4, %tmm31, %tmm6

// CHECK:      tdpbssde %tmm21, %tmm30, %tmm23
// CHECK: encoding: [0x62,0x82,0x57,0x00,0x5e,0xfe]
               tdpbssde %tmm21, %tmm30, %tmm23

// CHECK:      tdpbsude %tmm4, %tmm31, %tmm6
// CHECK: encoding: [0x62,0x92,0x5e,0x08,0x5e,0xf7]
               tdpbsude %tmm4, %tmm31, %tmm6

// CHECK:      tdpbsude %tmm21, %tmm30, %tmm23
// CHECK: encoding: [0x62,0x82,0x56,0x00,0x5e,0xfe]
               tdpbsude %tmm21, %tmm30, %tmm23

// CHECK:      tdpbusde %tmm4, %tmm31, %tmm6
// CHECK: encoding: [0x62,0x92,0x5d,0x08,0x5e,0xf7]
               tdpbusde %tmm4, %tmm31, %tmm6

// CHECK:      tdpbusde %tmm21, %tmm30, %tmm23
// CHECK: encoding: [0x62,0x82,0x55,0x00,0x5e,0xfe]
               tdpbusde %tmm21, %tmm30, %tmm23

// CHECK:      tdpbuude %tmm4, %tmm31, %tmm6
// CHECK: encoding: [0x62,0x92,0x5c,0x08,0x5e,0xf7]
               tdpbuude %tmm4, %tmm31, %tmm6

// CHECK:      tdpbuude %tmm21, %tmm30, %tmm23
// CHECK: encoding: [0x62,0x82,0x54,0x00,0x5e,0xfe]
               tdpbuude %tmm21, %tmm30, %tmm23

