; RUN: llc -O3 -disable-peephole -verify-machineinstrs -mcpu=sapphirerapids -mtriple=x86_64-unknown-unknown < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-unknown"

define <16 x float> @fmulcph(<16 x float> %a0, <16 x float> %a1) {
  ;CHECK-LABEL: fmulcph:
  ;CHECK:       vpxor %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %zmm1, %zmm0, %zmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fmulcph_mem(<16 x float> %a0, <16 x float>* %p1) {
  ;CHECK-LABEL: fmulcph_mem:
  ;CHECK:       vpxor %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi), %zmm0, %zmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <16 x float>, <16 x float>* %p1, align 64
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fmulcph_broadcast(<16 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fmulcph_broadcast:
  ;CHECK:       vpxor %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi){1to16}, %zmm0, %zmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <16 x float> undef, float %v1, i64 0
  %a1 = shufflevector <16 x float> %t0, <16 x float> undef, <16 x i32> zeroinitializer
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fmulcph_maskz(<16 x float> %a0, <16 x float> %a1, i16* %mask) {
  ;CHECK-LABEL: fmulcph_maskz:
  ;CHECK:       vpxor %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %zmm1, %zmm0, %zmm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i16, i16* %mask
  %3 = call <16 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> zeroinitializer, i16 %2, i32 4)
  ret <16 x float> %3
}

define <16 x float> @fcmulcph(<16 x float> %a0, <16 x float> %a1) {
  ;CHECK-LABEL: fcmulcph:
  ;CHECK:       vpxor %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %zmm1, %zmm0, %zmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fcmulcph_mem(<16 x float> %a0, <16 x float>* %p1) {
  ;CHECK-LABEL: fcmulcph_mem:
  ;CHECK:       vpxor %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi), %zmm0, %zmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <16 x float>, <16 x float>* %p1, align 64
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fcmulcph_broadcast(<16 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fcmulcph_broadcast:
  ;CHECK:       vpxor %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi){1to16}, %zmm0, %zmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <16 x float> undef, float %v1, i64 0
  %a1 = shufflevector <16 x float> %t0, <16 x float> undef, <16 x i32> zeroinitializer
  %2 = call <16 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> undef, i16 -1, i32 4)
  ret <16 x float> %2
}

define <16 x float> @fcmulcph_maskz(<16 x float> %a0, <16 x float> %a1, i16* %mask) {
  ;CHECK-LABEL: fcmulcph_maskz:
  ;CHECK:       vpxor %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %zmm1, %zmm0, %zmm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i16, i16* %mask
  %3 = call <16 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.512(<16 x float> %a0, <16 x float> %a1, <16 x float> zeroinitializer, i16 %2, i32 4)
  ret <16 x float> %3
}

define <4 x float> @fmulc(<4 x float> %a0, <4 x float> %a1) {
  ;CHECK-LABEL: fmulc:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %xmm1, %xmm0, %xmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fmulc_mem(<4 x float> %a0, <4 x float>* %p1) {
  ;CHECK-LABEL: fmulc_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi), %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <4 x float>, <4 x float>* %p1, align 64
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fmulc_broadcast(<4 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fmulc_broadcast:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi){1to4}, %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <4 x float> undef, float %v1, i64 0
  %a1 = shufflevector <4 x float> %t0, <4 x float> undef, <4 x i32> zeroinitializer
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fmulc_maskz(<4 x float> %a0, <4 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fmulc_maskz:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %xmm1, %xmm0, %xmm2 {%k1} {z}

  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> zeroinitializer, i8 %2)
  ret <4 x float> %3
}

define <4 x float> @fcmulc(<4 x float> %a0, <4 x float> %a1) {
  ;CHECK-LABEL: fcmulc:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %xmm1, %xmm0, %xmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fcmulc_mem(<4 x float> %a0, <4 x float>* %p1) {
  ;CHECK-LABEL: fcmulc_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi), %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <4 x float>, <4 x float>* %p1, align 64
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fcmulc_broadcast(<4 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fcmulc_broadcast:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi){1to4}, %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <4 x float> undef, float %v1, i64 0
  %a1 = shufflevector <4 x float> %t0, <4 x float> undef, <4 x i32> zeroinitializer
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1)
  ret <4 x float> %2
}

define <4 x float> @fcmulc_maskz(<4 x float> %a0, <4 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fcmulc_maskz:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %xmm1, %xmm0, %xmm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.128(<4 x float> %a0, <4 x float> %a1, <4 x float> zeroinitializer, i8 %2)
  ret <4 x float> %3
}

define <8 x float> @fmulc_ymm(<8 x float> %a0, <8 x float> %a1) {
  ;CHECK-LABEL: fmulc_ymm:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %ymm1, %ymm0, %ymm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fmulc_ymm_mem(<8 x float> %a0, <8 x float>* %p1) {
  ;CHECK-LABEL: fmulc_ymm_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi), %ymm0, %ymm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <8 x float>, <8 x float>* %p1, align 64
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fmulc_ymm_broadcast(<8 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fmulc_ymm_broadcast:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcph (%rdi){1to8}, %ymm0, %ymm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <8 x float> undef, float %v1, i64 0
  %a1 = shufflevector <8 x float> %t0, <8 x float> undef, <8 x i32> zeroinitializer
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fmulc_maskz_ymm(<8 x float> %a0, <8 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fmulc_maskz_ymm:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcph %ymm1, %ymm0, %ymm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <8 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> zeroinitializer, i8 %2)
  ret <8 x float> %3
}

define <8 x float> @fcmulc_ymm(<8 x float> %a0, <8 x float> %a1) {
  ;CHECK-LABEL: fcmulc_ymm:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %ymm1, %ymm0, %ymm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fcmulc_ymm_mem(<8 x float> %a0, <8 x float>* %p1) {
  ;CHECK-LABEL: fcmulc_ymm_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi), %ymm0, %ymm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <8 x float>, <8 x float>* %p1, align 64
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fcmulc_ymm_broadcast(<8 x float> %a0, float* %p1) {
  ;CHECK-LABEL: fcmulc_ymm_broadcast:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcph (%rdi){1to8}, %ymm0, %ymm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %v1 = load float, float* %p1, align 4
  %t0 = insertelement <8 x float> undef, float %v1, i64 0
  %a1 = shufflevector <8 x float> %t0, <8 x float> undef, <8 x i32> zeroinitializer
  %2 = call <8 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> undef, i8 -1)
  ret <8 x float> %2
}

define <8 x float> @fcmulc_maskz_ymm(<8 x float> %a0, <8 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fcmulc_maskz_ymm:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcph %ymm1, %ymm0, %ymm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <8 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.256(<8 x float> %a0, <8 x float> %a1, <8 x float> zeroinitializer, i8 %2)
  ret <8 x float> %3
}

define <4 x float> @fmulcsh(<4 x float> %a0, <4 x float> %a1) {
  ;CHECK-LABEL: fmulcsh:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcsh %xmm1, %xmm0, %xmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1, i32 4)
  ret <4 x float> %2
}

define <4 x float> @fmulcsh_mem(<4 x float> %a0, <4 x float>* %p1) {
  ;CHECK-LABEL: fmulcsh_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfmulcsh (%rdi), %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <4 x float>, <4 x float>* %p1, align 64
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1, i32 4)
  ret <4 x float> %2
}

define <4 x float> @fmulcsh_maskz(<4 x float> %a0, <4 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fmulcsh_maskz:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfmulcsh %xmm1, %xmm0, %xmm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <4 x float> @llvm.x86.avx512fp16.mask.vfmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> zeroinitializer, i8 %2, i32 4)
  ret <4 x float> %3
}

define <4 x float> @fcmulcsh(<4 x float> %a0, <4 x float> %a1) {
  ;CHECK-LABEL: fcmulcsh:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcsh %xmm1, %xmm0, %xmm2
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1, i32 4)
  ret <4 x float> %2
}

define <4 x float> @fcmulcsh_mem(<4 x float> %a0, <4 x float>* %p1) {
  ;CHECK-LABEL: fcmulcsh_mem:
  ;CHECK:       vxorps %xmm1, %xmm1, %xmm1
  ;CHECK-NEXT:  vfcmulcsh (%rdi), %xmm0, %xmm1
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm1},~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %a1 = load <4 x float>, <4 x float>* %p1, align 64
  %2 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> undef, i8 -1, i32 4)
  ret <4 x float> %2
}

define <4 x float> @fcmulcsh_maskz(<4 x float> %a0, <4 x float> %a1, i8* %mask) {
  ;CHECK-LABEL: fcmulcsh_maskz:
  ;CHECK:       vxorps %xmm2, %xmm2, %xmm2
  ;CHECK-NEXT:  vfcmulcsh %xmm1, %xmm0, %xmm2 {%k1} {z}
  %1 = tail call <2 x i64> asm sideeffect "nop", "=x,~{xmm2},~{xmm3},~{xmm4},~{xmm5},~{xmm6},~{xmm7},~{xmm8},~{xmm9},~{xmm10},~{xmm11},~{xmm12},~{xmm13},~{xmm14},~{xmm15},~{xmm16},~{xmm17},~{xmm18},~{xmm19},~{xmm20},~{xmm21},~{xmm22},~{xmm23},~{xmm24},~{xmm25},~{xmm26},~{xmm27},~{xmm28},~{xmm29},~{xmm30},~{xmm31},~{flags}"()
  %2 = load i8, i8* %mask
  %3 = call <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.csh(<4 x float> %a0, <4 x float> %a1, <4 x float> zeroinitializer, i8 %2, i32 4)
  ret <4 x float> %3
}

declare <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.csh(<4 x float>, <4 x float>, <4 x float>, i8, i32)
declare <4 x float> @llvm.x86.avx512fp16.mask.vfmul.csh(<4 x float>, <4 x float>, <4 x float>, i8, i32)
declare <16 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.512(<16 x float>, <16 x float>, <16 x float>, i16, i32)
declare <16 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.512(<16 x float>, <16 x float>, <16 x float>, i16, i32)
declare <8 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.256(<8 x float>, <8 x float>, <8 x float>, i8)
declare <8 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.256(<8 x float>, <8 x float>, <8 x float>, i8)
declare <4 x float> @llvm.x86.avx512fp16.mask.vfcmul.cph.128(<4 x float>, <4 x float>, <4 x float>, i8)
declare <4 x float> @llvm.x86.avx512fp16.mask.vfmul.cph.128(<4 x float>, <4 x float>, <4 x float>, i8)

