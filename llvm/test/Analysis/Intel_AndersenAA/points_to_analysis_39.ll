; This test verifies that "%i281 = load float" is NOT removed by
; gvn using Escape Analysis (Andersens's points-to analysis).
; AndersensAA can't handle "inttoptr (i64 add (i64 ptrtoint
; ([8 x [8 x float]]* @"f_80_$W" to i64), i64 72) to float*)"

; RUN: opt -opaque-pointers=0 < %s -passes='require<anders-aa>,gvn' -S 2>&1 | FileCheck %s

; CHECK: %i281 = load float, float


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"f_80_$W" = internal global [8 x [8 x float]] zeroinitializer
@GV = global float zeroinitializer

; Function Attrs: nounwind uwtable
define void @MAIN__() {
bb238:
  %i239 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) getelementptr inbounds ([8 x [8 x float]], [8 x [8 x float]]* @"f_80_$W", i64 0, i64 0, i64 0), i64 0)
  %i240 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* nonnull elementtype(float) %i239, i64 0)
  store float 0.000000e+00, float* getelementptr inbounds ([8 x [8 x float]], [8 x [8 x float]]* @"f_80_$W", i64 0, i64 0, i64 0), align 4
  %i279 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) inttoptr (i64 add (i64 ptrtoint ([8 x [8 x float]]* @"f_80_$W" to i64), i64 72) to float*), i64 0)
  %i280 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) %i279, i64 0)
  store float 0.000000e+00, float* %i280, align 1
  %i281 = load float, float* %i240, align 4
  store float %i281, float* @GV, align 8
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)
