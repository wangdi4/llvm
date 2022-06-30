# REQUIRES: intel_feature_isa_amx_tf32
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tmmultf32ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x51,0x6d,0xf4]
               tmmultf32ps tmm6, tmm5, tmm4

// CHECK:      tmmultf32ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x69,0x6d,0xd9]
               tmmultf32ps tmm3, tmm2, tmm1

// CHECK:      ttmmultf32ps tmm6, tmm5, tmm4
// CHECK: encoding: [0xc4,0xe2,0x52,0x6d,0xf4]
               ttmmultf32ps tmm6, tmm5, tmm4

// CHECK:      ttmmultf32ps tmm3, tmm2, tmm1
// CHECK: encoding: [0xc4,0xe2,0x6a,0x6d,0xd9]
               ttmmultf32ps tmm3, tmm2, tmm1

