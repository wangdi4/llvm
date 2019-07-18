; RUN: llc < %s | FileCheck %s
source_filename = "vecins.cpp"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable writeonly
define dso_local <2 x float> @_Z6vecinsf(float %x) local_unnamed_addr #0 {
; CHECK: sqrtf32
; CHECK: shufi32x2
; CHECK: shufi32x2
; CHECK: addf32x2
entry:
  %0 = tail call float @llvm.sqrt.f32(float %x)
  %vecinit2.i = insertelement <2 x float> <float undef, float 0.000000e+00>, float %0, i32 0
  %vecinit2.i8 = insertelement <2 x float> <float undef, float 0.000000e+00>, float %x, i32 0
  %1 = tail call <2 x float> @llvm.csa.addf32x2(<2 x float> %vecinit2.i, <2 x float> %vecinit2.i8, i8 0, i8 0, i8 0)
  ret <2 x float> %1
}

; Function Attrs: nounwind readnone speculatable
declare float @llvm.sqrt.f32(float) #1

; Function Attrs: nounwind readnone
declare <2 x float> @llvm.csa.addf32x2(<2 x float>, <2 x float>, i8, i8, i8) #2

attributes #0 = { nounwind uwtable writeonly }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind readnone }

