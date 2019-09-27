// REQUIRES: intel_feature_isa_amx2
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tdpbssde tmm6, tmm5, tmm4
// CHECK: encoding: [0x62,0xf2,0x5f,0x08,0x5e,0xf5]
               tdpbssde tmm6, tmm5, tmm4

// CHECK:      tdpbssde tmm3, tmm2, tmm1
// CHECK: encoding: [0x62,0xf2,0x77,0x08,0x5e,0xda]
               tdpbssde tmm3, tmm2, tmm1

// CHECK:      tdpbsude tmm6, tmm5, tmm4
// CHECK: encoding: [0x62,0xf2,0x5e,0x08,0x5e,0xf5]
               tdpbsude tmm6, tmm5, tmm4

// CHECK:      tdpbsude tmm3, tmm2, tmm1
// CHECK: encoding: [0x62,0xf2,0x76,0x08,0x5e,0xda]
               tdpbsude tmm3, tmm2, tmm1

// CHECK:      tdpbusde tmm6, tmm5, tmm4
// CHECK: encoding: [0x62,0xf2,0x5d,0x08,0x5e,0xf5]
               tdpbusde tmm6, tmm5, tmm4

// CHECK:      tdpbusde tmm3, tmm2, tmm1
// CHECK: encoding: [0x62,0xf2,0x75,0x08,0x5e,0xda]
               tdpbusde tmm3, tmm2, tmm1

// CHECK:      tdpbuude tmm6, tmm5, tmm4
// CHECK: encoding: [0x62,0xf2,0x5c,0x08,0x5e,0xf5]
               tdpbuude tmm6, tmm5, tmm4

// CHECK:      tdpbuude tmm3, tmm2, tmm1
// CHECK: encoding: [0x62,0xf2,0x74,0x08,0x5e,0xda]
               tdpbuude tmm3, tmm2, tmm1

// CHECK:      tdpbssde tmm22, tmm5, tmm4
// CHECK: encoding: [0x62,0xe2,0x5f,0x08,0x5e,0xf5]
               tdpbssde tmm22, tmm5, tmm4

// CHECK:      tdpbssde tmm19, tmm2, tmm1
// CHECK: encoding: [0x62,0xe2,0x77,0x08,0x5e,0xda]
               tdpbssde tmm19, tmm2, tmm1

// CHECK:      tdpbsude tmm22, tmm5, tmm4
// CHECK: encoding: [0x62,0xe2,0x5e,0x08,0x5e,0xf5]
               tdpbsude tmm22, tmm5, tmm4

// CHECK:      tdpbsude tmm19, tmm2, tmm1
// CHECK: encoding: [0x62,0xe2,0x76,0x08,0x5e,0xda]
               tdpbsude tmm19, tmm2, tmm1

// CHECK:      tdpbusde tmm22, tmm5, tmm4
// CHECK: encoding: [0x62,0xe2,0x5d,0x08,0x5e,0xf5]
               tdpbusde tmm22, tmm5, tmm4

// CHECK:      tdpbusde tmm19, tmm2, tmm1
// CHECK: encoding: [0x62,0xe2,0x75,0x08,0x5e,0xda]
               tdpbusde tmm19, tmm2, tmm1

// CHECK:      tdpbuude tmm22, tmm5, tmm4
// CHECK: encoding: [0x62,0xe2,0x5c,0x08,0x5e,0xf5]
               tdpbuude tmm22, tmm5, tmm4

// CHECK:      tdpbuude tmm19, tmm2, tmm1
// CHECK: encoding: [0x62,0xe2,0x74,0x08,0x5e,0xda]
               tdpbuude tmm19, tmm2, tmm1

// CHECK:      tdpbssde tmm6, tmm31, tmm4
// CHECK: encoding: [0x62,0x92,0x5f,0x08,0x5e,0xf7]
               tdpbssde tmm6, tmm31, tmm4

// CHECK:      tdpbssde tmm23, tmm30, tmm21
// CHECK: encoding: [0x62,0x82,0x57,0x00,0x5e,0xfe]
               tdpbssde tmm23, tmm30, tmm21

// CHECK:      tdpbsude tmm6, tmm31, tmm4
// CHECK: encoding: [0x62,0x92,0x5e,0x08,0x5e,0xf7]
               tdpbsude tmm6, tmm31, tmm4

// CHECK:      tdpbsude tmm23, tmm30, tmm21
// CHECK: encoding: [0x62,0x82,0x56,0x00,0x5e,0xfe]
               tdpbsude tmm23, tmm30, tmm21

// CHECK:      tdpbusde tmm6, tmm31, tmm4
// CHECK: encoding: [0x62,0x92,0x5d,0x08,0x5e,0xf7]
               tdpbusde tmm6, tmm31, tmm4

// CHECK:      tdpbusde tmm23, tmm30, tmm21
// CHECK: encoding: [0x62,0x82,0x55,0x00,0x5e,0xfe]
               tdpbusde tmm23, tmm30, tmm21

// CHECK:      tdpbuude tmm6, tmm31, tmm4
// CHECK: encoding: [0x62,0x92,0x5c,0x08,0x5e,0xf7]
               tdpbuude tmm6, tmm31, tmm4

// CHECK:      tdpbuude tmm23, tmm30, tmm21
// CHECK: encoding: [0x62,0x82,0x54,0x00,0x5e,0xfe]
               tdpbuude tmm23, tmm30, tmm21


