// REQUIRES: intel_feature_isa_amx_complex_evex
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s
// UXFAIL: *

// CHECK:      tcvtrowps2phie $123, %tmm4, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x5f,0x48,0x77,0xf7,0x7b]
               tcvtrowps2phie $123, %tmm4, %zmm23, %zmm22

// CHECK:      tcvtrowps2phie $123, %tmm1, %zmm23, %zmm22
// CHECK: encoding: [0x62,0xa3,0x77,0x48,0x77,0xf7,0x7b]
               tcvtrowps2phie $123, %tmm1, %zmm23, %zmm22

// CHECK:      tcvtrowps2phie %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf5]
               tcvtrowps2phie %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowps2phie %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf2]
               tcvtrowps2phie %ecx, %tmm2, %zmm22

