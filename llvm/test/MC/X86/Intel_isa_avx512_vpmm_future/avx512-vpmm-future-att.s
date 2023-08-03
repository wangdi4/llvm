// REQUIRES: intel_feature_isa_avx512_vpmm_future
// RUN: llvm-mc -triple x86_64-unknown-unknown --show-encoding < %s  | FileCheck %s

// CHECK: vmmxf16ps       %xmm4, %xmm3, %xmm0 # encoding: [0x62,0xf6,0x66,0x08,0x7f,0xc4]
     vmmxf16ps %xmm4, %xmm3, %xmm0

// CHECK: vmmxf16ps       %ymm4, %ymm3, %ymm0 # encoding: [0x62,0xf6,0x66,0x28,0x7f,0xc4]
     vmmxf16ps %ymm4, %ymm3, %ymm0

// CHECK: vmmxf16ps       %zmm4, %zmm3, %zmm0 # encoding: [0x62,0xf6,0x66,0x48,0x7f,0xc4]
     vmmxf16ps %zmm4, %zmm3, %zmm0

// CHECK: vmmif16ps       $0, %xmm4, %xmm3, %xmm0 # encoding: [0x62,0xf3,0x66,0x08,0x6f,0xc4,0x00]
     vmmif16ps $0x0, %xmm4, %xmm3, %xmm0

// CHECK: vmmif16ps       $0, %ymm4, %ymm3, %ymm0 # encoding: [0x62,0xf3,0x66,0x28,0x6f,0xc4,0x00]
     vmmif16ps $0x0, %ymm4, %ymm3, %ymm0

// CHECK: vmmif16ps       $0, %zmm4, %zmm3, %zmm0 # encoding: [0x62,0xf3,0x66,0x48,0x6f,0xc4,0x00]
     vmmif16ps $0x0, %zmm4, %zmm3, %zmm0

