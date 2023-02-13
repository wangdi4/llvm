// REQUIRES: intel_feature_isa_amx_complex
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK:      tcmmimfp16ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x59,0x6c,0xf5]
               tcmmimfp16ps %tmm4, %tmm5, %tmm6

// CHECK:      tcmmimfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x71,0x6c,0xda]
               tcmmimfp16ps %tmm1, %tmm2, %tmm3

// CHECK:      tcmmrlfp16ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x58,0x6c,0xf5]
               tcmmrlfp16ps %tmm4, %tmm5, %tmm6

// CHECK:      tcmmrlfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x70,0x6c,0xda]
               tcmmrlfp16ps %tmm1, %tmm2, %tmm3

// CHECK:      tconjtcmmimfp16ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x58,0x6b,0xf5]
               tconjtcmmimfp16ps %tmm4, %tmm5, %tmm6

// CHECK:      tconjtcmmimfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x70,0x6b,0xda]
               tconjtcmmimfp16ps %tmm1, %tmm2, %tmm3

// CHECK:      tconjtfp16 %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x79,0x6b,0xf5]
               tconjtfp16 %tmm5, %tmm6

// CHECK:      tconjtfp16 %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x79,0x6b,0xda]
               tconjtfp16 %tmm2, %tmm3

// CHECK:      ttcmmimfp16ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x5b,0x6b,0xf5]
               ttcmmimfp16ps %tmm4, %tmm5, %tmm6

// CHECK:      ttcmmimfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x73,0x6b,0xda]
               ttcmmimfp16ps %tmm1, %tmm2, %tmm3

// CHECK:      ttcmmrlfp16ps %tmm4, %tmm5, %tmm6
// CHECK: encoding: [0xc4,0xe2,0x5a,0x6b,0xf5]
               ttcmmrlfp16ps %tmm4, %tmm5, %tmm6

// CHECK:      ttcmmrlfp16ps %tmm1, %tmm2, %tmm3
// CHECK: encoding: [0xc4,0xe2,0x72,0x6b,0xda]
               ttcmmrlfp16ps %tmm1, %tmm2, %tmm3

