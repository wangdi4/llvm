// REQUIRES: intel_feature_isa_avx512_vpmm_future
// RUN: llvm-mc -triple x86_64-unknown-unknown -x86-asm-syntax=intel -output-asm-variant=1 --show-encoding %s | FileCheck %s

// CHECK: vmmxf16ps       xmm0, xmm3, xmm4        # encoding: [0x62,0xf6,0x66,0x08,0x7f,0xc4]
          vmmxf16ps xmm0, xmm3, xmm4

// CHECK: vmmxf16ps       ymm0, ymm3, ymm4        # encoding: [0x62,0xf6,0x66,0x28,0x7f,0xc4]
          vmmxf16ps ymm0, ymm3, ymm4

// CHECK: vmmxf16ps       zmm0, zmm3, zmm4        # encoding: [0x62,0xf6,0x66,0x48,0x7f,0xc4]
          vmmxf16ps zmm0, zmm3, zmm4

// CHECK: vmmif16ps       xmm0, xmm3, xmm4, 0     # encoding: [0x62,0xf3,0x66,0x08,0x6f,0xc4,0x00]
          vmmif16ps xmm0, xmm3, xmm4, 0

// CHECK: vmmif16ps       ymm0, ymm3, ymm4, 0     # encoding: [0x62,0xf3,0x66,0x28,0x6f,0xc4,0x00]
          vmmif16ps ymm0, ymm3, ymm4, 0

// CHECK: vmmif16ps       zmm0, zmm3, zmm4, 0     # encoding: [0x62,0xf3,0x66,0x48,0x6f,0xc4,0x00]
          vmmif16ps zmm0, zmm3, zmm4, 0
