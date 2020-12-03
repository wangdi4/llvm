// REQUIRES: intel_feature_isa_amx_complex
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tcmmimfp16ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x59,0x6c,0xf5]
               tcmmimfp16ps tmm6, tmm5, tmm4

// CHECK:      tcmmimfp16ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x71,0x6c,0xda]
               tcmmimfp16ps tmm3, tmm2, tmm1

// CHECK:      tcmmrlfp16ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x58,0x6c,0xf5]
               tcmmrlfp16ps tmm6, tmm5, tmm4

// CHECK:      tcmmrlfp16ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x70,0x6c,0xda]
               tcmmrlfp16ps tmm3, tmm2, tmm1

// CHECK:      tconjcmmimfp16ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x58,0x6b,0xf5]
               tconjcmmimfp16ps tmm6, tmm5, tmm4

// CHECK:      tconjcmmimfp16ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x70,0x6b,0xda]
               tconjcmmimfp16ps tmm3, tmm2, tmm1

// CHECK:      tconjfp16 tmm6, tmm5
// CHECK: encoding: [0xc4,0xe2,0x79,0x6b,0xf5]
               tconjfp16 tmm6, tmm5

// CHECK:      tconjfp16 tmm3, tmm2
// CHECK: encoding: [0xc4,0xe2,0x79,0x6b,0xda]
               tconjfp16 tmm3, tmm2

// CHECK:      ttcmmimfp16ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x5b,0x6b,0xf5]
               ttcmmimfp16ps tmm6, tmm5, tmm4

// CHECK:      ttcmmimfp16ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x73,0x6b,0xda]
               ttcmmimfp16ps tmm3, tmm2, tmm1

// CHECK:      ttcmmrlfp16ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x5a,0x6b,0xf5]
               ttcmmrlfp16ps tmm6, tmm5, tmm4

// CHECK:      ttcmmrlfp16ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x72,0x6b,0xda]
               ttcmmrlfp16ps tmm3, tmm2, tmm1

