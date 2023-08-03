// REQUIRES: intel_feature_isa_avx512_vpmm
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vmmf16ps xmm0, xmm3, xmm4
// CHECK: encoding: [0x62,0xf6,0x66,0x08,0x6f,0xc4]
          vmmf16ps xmm0, xmm3, xmm4

// CHECK: vmmf16ps ymm0, ymm3, ymm4
// CHECK: encoding: [0x62,0xf6,0x66,0x28,0x6f,0xc4]
          vmmf16ps ymm0, ymm3, ymm4

// CHECK: vmmf16ps zmm0, zmm3, zmm4
// CHECK: encoding: [0x62,0xf6,0x66,0x48,0x6f,0xc4]
          vmmf16ps zmm0, zmm3, zmm4

// CHECK: vmmbf16ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6f,0xf0]
          vmmbf16ps zmm22, zmm23, zmm24

// CHECK: vmmbf8ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x6d,0xf0]
          vmmbf8ps zmm22, zmm23, zmm24

// CHECK: vmmbhf8ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6d,0xf0]
          vmmbhf8ps zmm22, zmm23, zmm24

// CHECK: vmmhbf8ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x46,0x40,0x6d,0xf0]
          vmmhbf8ps zmm22, zmm23, zmm24

// CHECK: vmmhf8ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6d,0xf0]
          vmmhf8ps zmm22, zmm23, zmm24

// CHECK: vmmtf32ps zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6f,0xf0]
          vmmtf32ps zmm22, zmm23, zmm24

// CHECK: vpmmssbd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6c,0xf0]
          vpmmssbd zmm22, zmm23, zmm24

// CHECK: vpmmsubd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x46,0x40,0x6c,0xf0]
          vpmmsubd zmm22, zmm23, zmm24

// CHECK: vpmmusbd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6c,0xf0]
          vpmmusbd zmm22, zmm23, zmm24

// CHECK: vpmmuubd zmm22, zmm23, zmm24
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x6c,0xf0]
          vpmmuubd zmm22, zmm23, zmm24

// CHECK: vmmbf16ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6f,0xf0]
          vmmbf16ps ymm22, ymm23, ymm24

// CHECK: vmmbf16ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6f,0xf0]
          vmmbf16ps xmm22, xmm23, xmm24

// CHECK: vmmbf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x20,0x6d,0xf0]
          vmmbf8ps ymm22, ymm23, ymm24

// CHECK: vmmbf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x00,0x6d,0xf0]
          vmmbf8ps xmm22, xmm23, xmm24

// CHECK: vmmbhf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6d,0xf0]
          vmmbhf8ps ymm22, ymm23, ymm24

// CHECK: vmmbhf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6d,0xf0]
          vmmbhf8ps xmm22, xmm23, xmm24

// CHECK: vmmhbf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x46,0x20,0x6d,0xf0]
          vmmhbf8ps ymm22, ymm23, ymm24

// CHECK: vmmhbf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x46,0x00,0x6d,0xf0]
          vmmhbf8ps xmm22, xmm23, xmm24

// CHECK: vmmhf8ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6d,0xf0]
          vmmhf8ps ymm22, ymm23, ymm24

// CHECK: vmmhf8ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6d,0xf0]
          vmmhf8ps xmm22, xmm23, xmm24

// CHECK: vmmtf32ps ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6f,0xf0]
          vmmtf32ps ymm22, ymm23, ymm24

// CHECK: vmmtf32ps xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6f,0xf0]
          vmmtf32ps xmm22, xmm23, xmm24

// CHECK: vpmmssbd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6c,0xf0]
          vpmmssbd ymm22, ymm23, ymm24

// CHECK: vpmmssbd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6c,0xf0]
          vpmmssbd xmm22, xmm23, xmm24

// CHECK: vpmmsubd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x46,0x20,0x6c,0xf0]
          vpmmsubd ymm22, ymm23, ymm24

// CHECK: vpmmsubd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x46,0x00,0x6c,0xf0]
          vpmmsubd xmm22, xmm23, xmm24

// CHECK: vpmmusbd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6c,0xf0]
          vpmmusbd ymm22, ymm23, ymm24

// CHECK: vpmmusbd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6c,0xf0]
          vpmmusbd xmm22, xmm23, xmm24

// CHECK: vpmmuubd ymm22, ymm23, ymm24
// CHECK: encoding: [0x62,0x86,0x44,0x20,0x6c,0xf0]
          vpmmuubd ymm22, ymm23, ymm24

// CHECK: vpmmuubd xmm22, xmm23, xmm24
// CHECK: encoding: [0x62,0x86,0x44,0x00,0x6c,0xf0]
          vpmmuubd xmm22, xmm23, xmm24
