; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; This test case checks that the HIR verifier doesn't assert when checking
; the number of exits for the parent loop after unrolling the inner loop.

; BEGIN REGION { }
;       + DO i1 = 0, %m, 1   <DO_MULTI_EXIT_LOOP>
;       |   + DO i2 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
;       |   |   if ((%A)[i2 + 1] < 5)
;       |   |   {
;       |   |      if ((%A)[i2] > 10)
;       |   |      {
;       |   |         goto for.end.split.loop.exit;
;       |   |      }
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %m, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:       |   %tgu = (%n)/u2;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, %tgu + -1, 1   <DO_MULTI_EXIT_LOOP> <nounroll>
; CHECK:       |   |   if ((%A)[2 * i2 + 1] < 5)
; CHECK:       |   |   {
; CHECK:       |   |      if ((%A)[2 * i2] > 10)
; CHECK:       |   |      {
; CHECK:       |   |         goto for.end.split.loop.exit;
; CHECK:       |   |      }
; CHECK:       |   |   }
; CHECK:       |   |   if ((%A)[2 * i2 + 2] < 5)
; CHECK:       |   |   {
; CHECK:       |   |      if ((%A)[2 * i2 + 1] > 10)
; CHECK:       |   |      {
; CHECK:       |   |         goto for.end.split.loop.exit;
; CHECK:       |   |      }
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   if (2 * %tgu <u %n)
; CHECK:       |   {
; CHECK:       |      if ((%A)[2 * %tgu + 1] < 5)
; CHECK:       |      {
; CHECK:       |         if ((%A)[2 * %tgu] > 10)
; CHECK:       |         {
; CHECK:       |            goto for.end.split.loop.exit;
; CHECK:       |         }
; CHECK:       |      }
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local void @foo(ptr nocapture readonly %A, i64 %n, i64 %m) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.ph, label %for.end

for.body.ph:
  br label %for.outer.body

for.outer.body:
  %iv.outer = phi i64 [ 0, %for.body.ph ], [ %inc.outer, %for.inc5 ]
  br label %for.body

for.body:                                         ; preds = %for.body.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.outer.body ], [ %indvars.iv.next, %for.inc ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %0 = load i32, ptr %ptridx, align 4
  %cmp1 = icmp slt i32 %0, 5
  br i1 %cmp1, label %land.lhs.true, label %for.inc

land.lhs.true:                                    ; preds = %for.body
  %ptridx3 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %ptridx3, align 4
  %cmp4 = icmp sgt i32 %1, 10
  br i1 %cmp4, label %for.end.split.loop.exit, label %for.inc

for.inc:                                          ; preds = %for.body, %land.lhs.true
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.inc5, label %for.body, !llvm.loop !0

for.inc5:                                         ; preds = %for.body3
  %inc.outer = add nuw nsw i64 %iv.outer, 1
  %exitcond2 = icmp eq i64 %iv.outer, %m
  br i1 %exitcond2, label %for.end.loopexit, label %for.outer.body

for.end.split.loop.exit:                          ; preds = %land.lhs.true
  unreachable

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.end.split.loop.exit, %entry
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}