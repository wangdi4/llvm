; RUN: opt < %s -force-hir-cg -S -passes="hir-ssa-deconstruction,hir-cg" 2>&1 | FileCheck %s

; CHECK-LABEL: region.0
; CHECK: loop{{.*}}:
; CHECK-NEXT: %[[IV:.*]] = load i64, {{.*}} %i1.i64
; CHECK-NEXT: %[[P:.*]] = getelementptr inbounds [100 x float], {{.*}} %"foo_$X", i64 0, i64 %[[IV]]
; CHECK-NEXT: store float 1.500000e+01, {{.*}} %[[P]]


source_filename = "fortran-simple.f"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo_(ptr nocapture %"foo_$X") {
alloca:
  %ptr_cast = getelementptr inbounds [100 x float], ptr %"foo_$X", i64 0, i64 0
  br label %bb3

bb3:                                              ; preds = %bb3, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3 ], [ 1, %alloca ]
  %0 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) %ptr_cast, i64 %indvars.iv)
  store float 1.500000e+01, ptr %0, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond, label %bb1, label %bb3

bb1:                                              ; preds = %bb3
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

