// REQUIRES: intel_feature_isa_avx512_vpmm
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK: vmmxf16ps       %xmm4, %xmm3, %xmm0 # encoding: [0x62,0xf6,0x66,0x08,0x7f,0xc4]
     vmmxf16ps %xmm4, %xmm3, %xmm0

// CHECK: vmmxf16ps       %ymm4, %ymm3, %ymm0 # encoding: [0x62,0xf6,0x66,0x28,0x7f,0xc4]
     vmmxf16ps %ymm4, %ymm3, %ymm0

// CHECK: vmmxf16ps       %zmm4, %zmm3, %zmm0 # encoding: [0x62,0xf6,0x66,0x48,0x7f,0xc4]
     vmmxf16ps %zmm4, %zmm3, %zmm0
