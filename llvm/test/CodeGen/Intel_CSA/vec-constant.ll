; RUN: llc <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @RunReStream() {
entry:
; CHECK: mulf32x2
; CHECK-SAME: 0x3e9e01b3
; CHECK-SAME: 0x3f0000003e800000
  %a86 = tail call <2 x float> @llvm.csa.mulf32x2(<2 x float> <float 0x3FD3C03660000000, float undef>, <2 x float> <float 0.25, float 0.5>, i8 0, i8 0, i8 1)
  ret <2 x float> %a86
}

; Function Attrs: nounwind readnone
declare <2 x float> @llvm.csa.mulf32x2(<2 x float>, <2 x float>, i8, i8, i8) #0

attributes #0 = { nounwind readnone }
