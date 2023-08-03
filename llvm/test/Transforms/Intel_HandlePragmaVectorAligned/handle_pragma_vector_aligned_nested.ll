; RUN: opt -S -passes=handle-pragma-vector-aligned < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; void test_01(float *P1, double *P2, long M, long N) {
; #pragma vector aligned
;   for (long I = 0; I < M; ++I)
;     for (long J = 0; J < N; ++J)
;       P1[J] = P2[I];
; }
;
; Since the pragma is placed before the I loop, alignment assumptions
; are applicable to accesses indexed with I only. That is, P2[I] is
; considered aligned and P1[J] is not.
;
; CHECK-LABEL: @test_01
define void @test_01(ptr %P1, ptr %P2, i64 %M, i64 %N) {
; CHECK-LABEL: entry:
; CHECK-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr %P2, i64 16) ]
; CHECK-NEXT:    br label %for.outer
entry:
  br label %for.outer

for.outer:
  %I = phi i64 [ 0, %entry ], [ %I.inc, %for.outer.latch ]
  br label %for.inner

for.inner:
  %J = phi i64 [ 0, %for.outer ], [ %J.inc, %for.inner ]
  %src = getelementptr float, ptr %P2, i64 %I
  %dst = getelementptr float, ptr %P1, i64 %J
  %val = load float, ptr %src
  store float %val, ptr %dst
  %J.inc = add nuw nsw i64 %J, 1
  %cmp.inner = icmp slt i64 %J.inc, %N
  br i1 %cmp.inner, label %for.inner, label %for.outer.latch

for.outer.latch:
  %I.inc = add nuw nsw i64 %I, 1
  %cmp.outer = icmp slt i64 %I.inc, %M
  br i1 %cmp.outer, label %for.outer, label %exit, !llvm.loop !0

exit:
  ret void
}

; void test_02(float *P1, double *P2, long M, long N) {
;   for (long I = 0; I < M; ++I)
; #pragma vector aligned
;     for (long J = 0; J < N; ++J)
;       P1[J] = P2[I];
; }
;
; Since the pragma is placed before the J loop, alignment assumptions
; are applicable to accesses indexed with J only. That is, P1[J] is
; considered aligned and P2[I] is not.
;
; CHECK-LABEL: @test_02
define void @test_02(ptr %P1, ptr %P2, i64 %M, i64 %N) {
entry:
  br label %for.outer

; CHECK-LABEL: for.outer:
; CHECK-NEXT:    %I = phi i64 [ 0, %entry ], [ %I.inc, %for.outer.latch ]
; CHECK-NEXT:    call void @llvm.assume(i1 true) [ "align"(ptr %P1, i64 16) ]
; CHECK-NEXT:    br label %for.inner
for.outer:
  %I = phi i64 [ 0, %entry ], [ %I.inc, %for.outer.latch ]
  br label %for.inner

for.inner:
  %J = phi i64 [ 0, %for.outer ], [ %J.inc, %for.inner ]
  %src = getelementptr float, ptr %P2, i64 %I
  %dst = getelementptr float, ptr %P1, i64 %J
  %val = load float, ptr %src
  store float %val, ptr %dst
  %J.inc = add nuw nsw i64 %J, 1
  %cmp.inner = icmp slt i64 %J.inc, %N
  br i1 %cmp.inner, label %for.inner, label %for.outer.latch, !llvm.loop !0

for.outer.latch:
  %I.inc = add nuw nsw i64 %I, 1
  %cmp.outer = icmp slt i64 %I.inc, %M
  br i1 %cmp.outer, label %for.outer, label %exit

exit:
  ret void
}

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.mustprogress"}
!2 = !{!"llvm.loop.intel.vector.aligned"}
