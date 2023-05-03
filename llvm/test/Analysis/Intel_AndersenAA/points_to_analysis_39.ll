; This test verifies that "%i281 = load float" is NOT removed by
; gvn using Escape Analysis (Andersens's points-to analysis).
; AndersensAA can't handle "inttoptr (i64 add (i64 ptrtoint
; (ptr @"f_80_$W" to i64), i64 72) to ptr)"

; RUN: opt < %s -passes='require<anders-aa>,gvn' -S 2>&1 | FileCheck %s

; CHECK: %i281 = load float, ptr


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"f_80_$W" = internal global [8 x [8 x float]] zeroinitializer
@GV = global float 0.000000e+00

define void @MAIN__() {
bb238:
  %i239 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(float) @"f_80_$W", i64 0)
  %i240 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(float) %i239, i64 0)
  store float 0.000000e+00, ptr @"f_80_$W", align 4
  %i279 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(float) inttoptr (i64 add (i64 ptrtoint (ptr @"f_80_$W" to i64), i64 72) to ptr), i64 0)
  %i280 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(float) %i279, i64 0)
  store float 0.000000e+00, ptr %i280, align 1
  %i281 = load float, ptr %i240, align 4
  store float %i281, ptr @GV, align 8
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }