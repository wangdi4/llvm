; RUN: opt -S -passes=handle-pragma-vector-aligned < %s | FileCheck %s

; Tests in this file are not supposed to trigger generation of any
; alignment assumptions.
; CHECK-NOT: assume

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test_non_addrec(ptr %P1, i64 %N) {
entry:
  br label %for.body

for.body:
  %I = phi i64 [ 0, %entry ], [ %I.inc, %for.body ]
  ;; Address is not a SCEVAddRecExpr.
  store i64 %I, ptr %P1
  %I.inc = add nuw nsw i64 %I, 1
  %cmp = icmp sle i64 %I.inc, %N
  br i1 %cmp, label %for.body, label %exit, !llvm.loop !0

exit:
  ret void
}

define void @test_reverse(ptr %P1, ptr %P2, i64 %N) {
entry:
  br label %for.body

for.body:
  %I = phi i64 [ 0, %entry ], [ %I.inc, %for.body ]
  %idx = sub nuw nsw i64 0, %I
  %src = getelementptr float, ptr %P2, i64 %idx
  %dst = getelementptr float, ptr %P1, i64 %idx
  ;; Both %src and %dst are not unit-stride pointers (stride is equal
  ;; to -4 instead of 4).
  %val = load float, ptr %src
  store float %val, ptr %dst
  %I.inc = add nuw nsw i64 %I, 1
  %cmp = icmp sle i64 %I.inc, %N
  br i1 %cmp, label %for.body, label %exit, !llvm.loop !0

exit:
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.vector.aligned"}
