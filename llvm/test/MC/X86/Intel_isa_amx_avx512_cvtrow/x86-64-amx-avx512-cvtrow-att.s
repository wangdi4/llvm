# REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK:      tcvtrowd2pse %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x4a,0xf5]
               tcvtrowd2pse %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowd2pse %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x4a,0xf2]
               tcvtrowd2pse %ecx, %tmm2, %zmm22

// CHECK:      tcvtrowd2pse $123, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x07,0xf5,0x7b]
               tcvtrowd2pse $123, %tmm5, %zmm22

// CHECK:      tcvtrowd2pse $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x07,0xf2,0x7b]
               tcvtrowd2pse $123, %tmm2, %zmm22

// CHECK:      tcvtrowps2pbf16he %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x77,0x48,0x6d,0xf5]
               tcvtrowps2pbf16he %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowps2pbf16he %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x77,0x48,0x6d,0xf2]
               tcvtrowps2pbf16he %ecx, %tmm2, %zmm22

// CHECK:      tcvtrowps2pbf16he $123, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x07,0xf5,0x7b]
               tcvtrowps2pbf16he $123, %tmm5, %zmm22

// CHECK:      tcvtrowps2pbf16he $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x07,0xf2,0x7b]
               tcvtrowps2pbf16he $123, %tmm2, %zmm22

// CHECK:      tcvtrowps2pbf16le %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x6d,0xf5]
               tcvtrowps2pbf16le %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowps2pbf16le %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x6d,0xf2]
               tcvtrowps2pbf16le %ecx, %tmm2, %zmm22

// CHECK:      tcvtrowps2pbf16le $123, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x77,0xf5,0x7b]
               tcvtrowps2pbf16le $123, %tmm5, %zmm22

// CHECK:      tcvtrowps2pbf16le $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x77,0xf2,0x7b]
               tcvtrowps2pbf16le $123, %tmm2, %zmm22

// CHECK:      tcvtrowps2phhe %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x74,0x48,0x6d,0xf5]
               tcvtrowps2phhe %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowps2phhe %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x74,0x48,0x6d,0xf2]
               tcvtrowps2phhe %ecx, %tmm2, %zmm22

// CHECK:      tcvtrowps2phhe $123, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x07,0xf5,0x7b]
               tcvtrowps2phhe $123, %tmm5, %zmm22

// CHECK:      tcvtrowps2phhe $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x07,0xf2,0x7b]
               tcvtrowps2phhe $123, %tmm2, %zmm22

// CHECK:      tcvtrowps2phle %ecx, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf5]
               tcvtrowps2phle %ecx, %tmm5, %zmm22

// CHECK:      tcvtrowps2phle %ecx, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf2]
               tcvtrowps2phle %ecx, %tmm2, %zmm22

// CHECK:      tcvtrowps2phle $123, %tmm5, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x77,0xf5,0x7b]
               tcvtrowps2phle $123, %tmm5, %zmm22

// CHECK:      tcvtrowps2phle $123, %tmm2, %zmm22
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x77,0xf2,0x7b]
               tcvtrowps2phle $123, %tmm2, %zmm22

