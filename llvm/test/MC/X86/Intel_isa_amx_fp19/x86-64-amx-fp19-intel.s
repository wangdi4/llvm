# REQUIRES: intel_feature_isa_amx_fp19
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tmmulfp19ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x6d,0xf4]
               tmmulfp19ps tmm6, tmm5, tmm4

// CHECK:      tmmulfp19ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x69,0x6d,0xd9]
               tmmulfp19ps tmm3, tmm2, tmm1

// CHECK:      ttmmulfp19ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x52,0x6d,0xf4]
               ttmmulfp19ps tmm6, tmm5, tmm4

// CHECK:      ttmmulfp19ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x6a,0x6d,0xd9]
               ttmmulfp19ps tmm3, tmm2, tmm1

