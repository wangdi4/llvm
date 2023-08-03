// REQUIRES: intel_feature_isa_avx512_vpmm
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK: vmmf16ps %xmm4, %xmm3, %xmm0
// CHECK: encoding: [0x62,0xf6,0x66,0x08,0x6f,0xc4]
          vmmf16ps %xmm4, %xmm3, %xmm0

// CHECK: vmmf16ps %ymm4, %ymm3, %ymm0
// CHECK: encoding: [0x62,0xf6,0x66,0x28,0x6f,0xc4]
          vmmf16ps %ymm4, %ymm3, %ymm0

// CHECK: vmmf16ps %zmm4, %zmm3, %zmm0
// CHECK: encoding: [0x62,0xf6,0x66,0x48,0x6f,0xc4]
          vmmf16ps %zmm4, %zmm3, %zmm0

// CHECK: vmmbf16ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6f,0xf0]
          vmmbf16ps %zmm24, %zmm23, %zmm22

// CHECK: vmmbf8ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x6d,0xf0]
          vmmbf8ps %zmm24, %zmm23, %zmm22

// CHECK: vmmbhf8ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6d,0xf0]
          vmmbhf8ps %zmm24, %zmm23, %zmm22

// CHECK: vmmhbf8ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x46,0x40,0x6d,0xf0]
          vmmhbf8ps %zmm24, %zmm23, %zmm22

// CHECK: vmmhf8ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6d,0xf0]
          vmmhf8ps %zmm24, %zmm23, %zmm22

// CHECK: vmmtf32ps %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6f,0xf0]
          vmmtf32ps %zmm24, %zmm23, %zmm22

// CHECK: vpmmssbd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x47,0x40,0x6c,0xf0]
          vpmmssbd %zmm24, %zmm23, %zmm22

// CHECK: vpmmsubd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x46,0x40,0x6c,0xf0]
          vpmmsubd %zmm24, %zmm23, %zmm22

// CHECK: vpmmusbd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x45,0x40,0x6c,0xf0]
          vpmmusbd %zmm24, %zmm23, %zmm22

// CHECK: vpmmuubd %zmm24, %zmm23, %zmm22
// CHECK: encoding: [0x62,0x86,0x44,0x40,0x6c,0xf0]
          vpmmuubd %zmm24, %zmm23, %zmm22

// CHECK: vmmbf16ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6f,0xf0]
          vmmbf16ps %ymm24, %ymm23, %ymm22

// CHECK: vmmbf16ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6f,0xf0]
          vmmbf16ps %xmm24, %xmm23, %xmm22

// CHECK: vmmbf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x44,0x20,0x6d,0xf0]
          vmmbf8ps %ymm24, %ymm23, %ymm22

// CHECK: vmmbf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x44,0x00,0x6d,0xf0]
          vmmbf8ps %xmm24, %xmm23, %xmm22

// CHECK: vmmbhf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6d,0xf0]
          vmmbhf8ps %ymm24, %ymm23, %ymm22

// CHECK: vmmbhf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6d,0xf0]
          vmmbhf8ps %xmm24, %xmm23, %xmm22

// CHECK: vmmhbf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x46,0x20,0x6d,0xf0]
          vmmhbf8ps %ymm24, %ymm23, %ymm22

// CHECK: vmmhbf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x46,0x00,0x6d,0xf0]
          vmmhbf8ps %xmm24, %xmm23, %xmm22

// CHECK: vmmhf8ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6d,0xf0]
          vmmhf8ps %ymm24, %ymm23, %ymm22

// CHECK: vmmhf8ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6d,0xf0]
          vmmhf8ps %xmm24, %xmm23, %xmm22

// CHECK: vmmtf32ps %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6f,0xf0]
          vmmtf32ps %ymm24, %ymm23, %ymm22

// CHECK: vmmtf32ps %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6f,0xf0]
          vmmtf32ps %xmm24, %xmm23, %xmm22

// CHECK: vpmmssbd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x47,0x20,0x6c,0xf0]
          vpmmssbd %ymm24, %ymm23, %ymm22

// CHECK: vpmmssbd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x47,0x00,0x6c,0xf0]
          vpmmssbd %xmm24, %xmm23, %xmm22

// CHECK: vpmmsubd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x46,0x20,0x6c,0xf0]
          vpmmsubd %ymm24, %ymm23, %ymm22

// CHECK: vpmmsubd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x46,0x00,0x6c,0xf0]
          vpmmsubd %xmm24, %xmm23, %xmm22

// CHECK: vpmmusbd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x45,0x20,0x6c,0xf0]
          vpmmusbd %ymm24, %ymm23, %ymm22

// CHECK: vpmmusbd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x45,0x00,0x6c,0xf0]
          vpmmusbd %xmm24, %xmm23, %xmm22

// CHECK: vpmmuubd %ymm24, %ymm23, %ymm22
// CHECK: encoding: [0x62,0x86,0x44,0x20,0x6c,0xf0]
          vpmmuubd %ymm24, %ymm23, %ymm22

// CHECK: vpmmuubd %xmm24, %xmm23, %xmm22
// CHECK: encoding: [0x62,0x86,0x44,0x00,0x6c,0xf0]
          vpmmuubd %xmm24, %xmm23, %xmm22
