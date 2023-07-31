; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that we do not unroll deeply nested multi-exit loops with liveouts
; even if the loop body is small if it doesn't pass the additional reuse
; profitability check.

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %indvars.iv.out = i1;
; CHECK: |   if ((%A)[i1 + 1] < 5)
; CHECK: |   {
; CHECK: |      goto for.end.split.loop.exit;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A, i64 %n) {
entry:
  %cmp = icmp sgt i64 %n, 0
  br label %outer.loop2

outer.loop2:
  %outer.iv2 = phi i64 [ %n , %entry ], [ %outer.iv.inc2, %outer.latch2 ]
  br label %outer.loop1

outer.loop1:
  %outer.iv1 = phi i64 [ %n , %outer.loop2 ], [ %outer.iv.inc1, %outer.latch1 ]
  br label %outer.loop

outer.loop:
  %outer.iv = phi i64 [ %n , %outer.loop1 ], [ %outer.iv.inc, %outer.latch ]
  br i1 %cmp, label %for.body.ph, label %outer.latch

for.body.ph:
  br label %for.body

for.body:                                         ; preds = %for.body.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.ph ], [ %indvars.iv.next, %for.inc ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv.next
  %0 = load i32, ptr %ptridx, align 4
  %cmp1 = icmp slt i32 %0, 5
  br i1 %cmp1, label %for.end.split.loop.exit, label %for.inc

for.inc:                                          ; preds = %for.body
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.split.loop.exit:                          ; preds = %for.body
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.body ]
  %1 = trunc i64 %indvars.iv.lcssa to i32
  br label %outer.latch

for.end.loopexit:                                 ; preds = %for.inc
  br label %outer.latch

outer.latch:
  %i.0.lcssa = phi i32 [ %1, %for.end.split.loop.exit ], [ 10, %for.end.loopexit ], [ 0, %outer.loop ]
  %outer.iv.inc = sdiv i64 %outer.iv, 2
  %cmp5 = icmp sgt i64 %outer.iv.inc, 1
  br i1 %cmp5, label %outer.loop, label %outer.latch1

outer.latch1:
  %i.0.lcssa1 = phi i32 [ %i.0.lcssa, %outer.latch ]
  %outer.iv.inc1 = sdiv i64 %outer.iv1, 2
  %cmp6 = icmp sgt i64 %outer.iv.inc1, 1
  br i1 %cmp6, label %outer.loop1, label %outer.latch2

outer.latch2:
  %i.0.lcssa2 = phi i32 [ %i.0.lcssa1, %outer.latch1 ]
  %outer.iv.inc2 = sdiv i64 %outer.iv2, 2
  %cmp7 = icmp sgt i64 %outer.iv.inc2, 1
  br i1 %cmp7, label %outer.loop2, label %exit

exit:                                          ; preds = %outer.latch2
  ret i32 %i.0.lcssa2
}


