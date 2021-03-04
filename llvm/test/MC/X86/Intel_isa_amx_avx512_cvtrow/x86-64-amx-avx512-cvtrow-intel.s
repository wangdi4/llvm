# REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK:      tcvtrowd2pse zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x4a,0xf5]
               tcvtrowd2pse zmm22, tmm5, ecx

// CHECK:      tcvtrowd2pse zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x4a,0xf2]
               tcvtrowd2pse zmm22, tmm2, ecx

// CHECK:      tcvtrowd2pse zmm22, tmm5, 123
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x07,0xf5,0x7b]
               tcvtrowd2pse zmm22, tmm5, 123

// CHECK:      tcvtrowd2pse zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x07,0xf2,0x7b]
               tcvtrowd2pse zmm22, tmm2, 123

// CHECK:      tcvtrowps2pbf16he zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x77,0x48,0x6d,0xf5]
               tcvtrowps2pbf16he zmm22, tmm5, ecx

// CHECK:      tcvtrowps2pbf16he zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x77,0x48,0x6d,0xf2]
               tcvtrowps2pbf16he zmm22, tmm2, ecx

// CHECK:      tcvtrowps2pbf16he zmm22, tmm5, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x07,0xf5,0x7b]
               tcvtrowps2pbf16he zmm22, tmm5, 123

// CHECK:      tcvtrowps2pbf16he zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x07,0xf2,0x7b]
               tcvtrowps2pbf16he zmm22, tmm2, 123

// CHECK:      tcvtrowps2pbf16le zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x6d,0xf5]
               tcvtrowps2pbf16le zmm22, tmm5, ecx

// CHECK:      tcvtrowps2pbf16le zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x76,0x48,0x6d,0xf2]
               tcvtrowps2pbf16le zmm22, tmm2, ecx

// CHECK:      tcvtrowps2pbf16le zmm22, tmm5, 123
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x77,0xf5,0x7b]
               tcvtrowps2pbf16le zmm22, tmm5, 123

// CHECK:      tcvtrowps2pbf16le zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7e,0x48,0x77,0xf2,0x7b]
               tcvtrowps2pbf16le zmm22, tmm2, 123

// CHECK:      tcvtrowps2phhe zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x74,0x48,0x6d,0xf5]
               tcvtrowps2phhe zmm22, tmm5, ecx

// CHECK:      tcvtrowps2phhe zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x74,0x48,0x6d,0xf2]
               tcvtrowps2phhe zmm22, tmm2, ecx

// CHECK:      tcvtrowps2phhe zmm22, tmm5, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x07,0xf5,0x7b]
               tcvtrowps2phhe zmm22, tmm5, 123

// CHECK:      tcvtrowps2phhe zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7c,0x48,0x07,0xf2,0x7b]
               tcvtrowps2phhe zmm22, tmm2, 123

// CHECK:      tcvtrowps2phle zmm22, tmm5, ecx
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf5]
               tcvtrowps2phle zmm22, tmm5, ecx

// CHECK:      tcvtrowps2phle zmm22, tmm2, ecx
// CHECK: encoding: [0x62,0xe2,0x75,0x48,0x6d,0xf2]
               tcvtrowps2phle zmm22, tmm2, ecx

// CHECK:      tcvtrowps2phle zmm22, tmm5, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x77,0xf5,0x7b]
               tcvtrowps2phle zmm22, tmm5, 123

// CHECK:      tcvtrowps2phle zmm22, tmm2, 123
// CHECK: encoding: [0x62,0xe3,0x7f,0x48,0x77,0xf2,0x7b]
               tcvtrowps2phle zmm22, tmm2, 123

