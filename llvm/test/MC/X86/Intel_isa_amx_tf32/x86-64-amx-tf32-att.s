# REQUIRES: intel_feature_isa_amx_tf32
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK:      tmmultf32ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x51,0x6d,0xf4]
               tmmultf32ps %tmm4, %tmm5, %tmm6

// CHECK:      tmmultf32ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x69,0x6d,0xd9]
               tmmultf32ps %tmm1, %tmm2, %tmm3

// CHECK:      ttmmultf32ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x52,0x6d,0xf4]
               ttmmultf32ps %tmm4, %tmm5, %tmm6

// CHECK:      ttmmultf32ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x6a,0x6d,0xd9]
               ttmmultf32ps %tmm1, %tmm2, %tmm3

