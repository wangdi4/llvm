; REQUIRES: asserts
; RUN: opt < %s -hir-details-dims -hir-ssa-deconstruction -hir-framework -analyze | FileCheck %s
; RUN: opt < %s -hir-details-dims -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; RUN: opt < %s -hir-region-identification -hir-region-print-cost-model-stats -debug-only=hir-region-identification 2>&1 | FileCheck %s --check-prefix=STATS

; Verify that subscript intrinsic is ignored by the framework cost model when counting instructions.
; STATS: Loop instruction count: 1

; Fortran:
;
;  SUBROUTINE FOO(X)
;    real, dimension(100) :: X
;
;    DO i = 1, 100
;      X(i) = 15.0
;    END DO
;  END

; CHECK-LABEL: BEGIN REGION { }
; CHECK:         + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:    |   (%"foo_$X")[0:0:400([100 x float]*:0)][0:i1:4([100 x float]:100)] = 1.500000e+01;
; CHECK-NEXT:    + END LOOP
; CHECK-NEXT:  END REGION

source_filename = "fortran-simple.f"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_([100 x float]* nocapture %"foo_$X") {
alloca:
  %ptr_cast = getelementptr inbounds [100 x float], [100 x float]* %"foo_$X", i64 0, i64 0
  br label %bb3

bb3:                                              ; preds = %bb3, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3 ], [ 1, %alloca ]
  %0 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* %ptr_cast, i64 %indvars.iv)
  store float 1.500000e+01, float* %0, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %bb1, label %bb3

bb1:                                              ; preds = %bb3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

