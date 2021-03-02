// REQUIRES: intel_feature_isa_amx_complex_evex
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s
// UXFAIL: *

// CHECK:      tcvtrowps2phie zmm22, zmm23, tmm4, 123
// CHECK: encoding: [0x62,0xa3,0x5f,0x48,0x77,0xf7,0x7b]
               tcvtrowps2phie zmm22, zmm23, tmm4, 123

// CHECK:      tcvtrowps2phie zmm22, zmm23, tmm1, 123
// CHECK: encoding: [0x62,0xa3,0x77,0x48,0x77,0xf7,0x7b]
               tcvtrowps2phie zmm22, zmm23, tmm1, 123

// CHECK:      tcvtrowps2phie zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf5]
               tcvtrowps2phie zmm22, tmm5, ecx

// CHECK:      tcvtrowps2phie zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf2]
               tcvtrowps2phie zmm22, tmm2, ecx

