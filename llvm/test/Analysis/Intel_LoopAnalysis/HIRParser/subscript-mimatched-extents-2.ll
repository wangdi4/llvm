; REQUIRES: asserts
; RUN: opt < %s -hir-details-dims -passes="hir-ssa-deconstruction,print<hir>" -disable-output 2>&1 | FileCheck %s

; Verify that in the presence of conflicting extent metadata in the IR (10 vs 20),
; we pick up the smaller info.

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%p)[0:i1 + %t + -1:4(float:10)] = 1.500000e+01;
; CHECK: + END LOOP


source_filename = "fortran-simple.f"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr nocapture %p, i64 %t) {
alloca:
  br label %bb3

bb3:                                              ; preds = %bb3, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3 ], [ 1, %alloca ]
  %sub1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %p, i64 %t), !ifx.array_extent !0
  %sub2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %sub1, i64 %indvars.iv), !ifx.array_extent !1
  store float 1.500000e+01, ptr %sub2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %bb1, label %bb3

bb1:                                              ; preds = %bb3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!0 = !{i64 20}
!1 = !{i64 10}
