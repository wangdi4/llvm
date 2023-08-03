; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" 2>&1 < %s | FileCheck %s


; Input HIR-
; + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
; |   %indvars.iv.out = i1;
; |   if ((%A)[i1 + 1] < 5)
; |   {
; |      if ((%A)[i1] > 10)
; |      {
; |         goto for.end.split.loop.exit;
; |      }
; |   }
; + END LOOP

; Verify that do-multi-exit loop is unrolled by a high unroll factor of 8
; because temporal locality exposes redundant loads.

; The modified loop below only shows the first iteration for brevity.

; CHECK: BEGIN REGION { modified }

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_MULTI_EXIT_LOOP> <nounroll>
; CHECK: |   %indvars.iv.out = 8 * i1;
; CHECK: |   if ((%A)[8 * i1 + 1] < 5)
; CHECK: |   {
; CHECK: |      if ((%A)[8 * i1] > 10)
; CHECK: |      {
; CHECK: |         goto for.end.split.loop.exit;
; CHECK: |      }
; CHECK: |   }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A, i64 %n) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.ph, label %for.end

for.body.ph:
  br label %for.body

for.body:                                         ; preds = %for.body.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.ph ], [ %indvars.iv.next, %for.inc ]
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
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.split.loop.exit:                          ; preds = %land.lhs.true
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %land.lhs.true ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  br label %for.end

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.end.split.loop.exit, %entry
  %i.0.lcssa = phi i32 [ %2, %for.end.split.loop.exit ], [ 10, %for.end.loopexit ], [ 0, %entry ]
  ret i32 %i.0.lcssa
}

