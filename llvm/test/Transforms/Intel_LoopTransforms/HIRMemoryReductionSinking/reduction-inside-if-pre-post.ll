; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-memory-reduction-sinking,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the pre-header and post-exit instructions in
; the inner loop are moved before and after the loop when the reduction
; inside the condition is transformed. The ZTT should be extracted too.

; Before transformation

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:       |      %ld1 = (%a)[i1];
; CHECK-NEXT:       |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>
; CHECK-NEXT:       |   |   %hir.de.ssa.copy0.out = 3 * i2 + %ld1;
; CHECK-NEXT:       |   |   if ((%b)[i1] <= 10)
; CHECK-NEXT:       |   |   {
; CHECK-NEXT:       |   |      %3 = (%a)[5];
; CHECK-NEXT:       |   |      (%a)[5] = %3 + 2;
; CHECK-NEXT:       |   |   }
; CHECK-NEXT:       |   + END LOOP
; CHECK-NEXT:       |      (%a)[i1] = %hir.de.ssa.copy0.out;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION


; After transformation

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:       |   if (%m > 0)
; CHECK-NEXT:       |   {
; CHECK-NEXT:       |      %ld1 = (%a)[i1];
; CHECK-NEXT:       |
; CHECK-NEXT:       |         %tmp = 0;
; CHECK-NEXT:       |      + DO i2 = 0, %m + -1, 1   <DO_LOOP>
; CHECK-NEXT:       |      |   %hir.de.ssa.copy0.out = 3 * i2 + %ld1;
; CHECK-NEXT:       |      |   if ((%b)[i1] <= 10)
; CHECK-NEXT:       |      |   {
; CHECK-NEXT:       |      |      %tmp = %tmp  +  2;
; CHECK-NEXT:       |      |   }
; CHECK-NEXT:       |      + END LOOP
; CHECK-NEXT:       |
; CHECK-NEXT:       |      if (%tmp != 0)
; CHECK-NEXT:       |      {
; CHECK-NEXT:       |         %3 = (%a)[5];
; CHECK-NEXT:       |         (%a)[5] = %3 + %tmp;
; CHECK-NEXT:       |      }
; CHECK-NEXT:       |      (%a)[i1] = %hir.de.ssa.copy0.out;
; CHECK-NEXT:       |   }
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture noalias noundef %a, ptr nocapture noundef readonly %b, i64 %m) local_unnamed_addr #0 {
entry:
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 5
  %j.linear.iv = alloca i32, align 4
  br label %outer.loop.ph

outer.loop.ph:
  br label %outer.loop.for.body

outer.loop.for.body:
  %indvars.iv = phi i64 [ 0, %outer.loop.ph ], [ %inc, %outer.loop.end ]
  %cmp2 = icmp sgt i64 %m, 0
  br i1 %cmp2, label %inner.for.body.lr.ph, label %outer.loop.end

inner.for.body.lr.ph:
  %0 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  %ld1 = load i32, ptr %0
  br label %inner.for.body

inner.for.body:                               ; preds = %inner.for.inc, %inner.for.body.lr.ph
  %indvars.iv2 = phi i64 [ %indvars.iv.next, %inner.for.inc ], [ 0, %inner.for.body.lr.ph ]
  %1 = phi i32 [ %add, %inner.for.inc ], [ %ld1, %inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx
  %cmp1 = icmp sgt i32 %2, 10
  br i1 %cmp1, label %inner.for.inc, label %if.then

if.then:                                          ; preds = %inner.for.body
  %3 = load i32, ptr %arrayidx2
  %add2 = add nsw i32 %3, 2
  store i32 %add2, ptr %arrayidx2
  br label %inner.for.inc

inner.for.inc:                                ; preds = %inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv2, 1
  %add = add nuw nsw i32 %1, 3
  %exitcond = icmp eq i64 %indvars.iv.next, %m
  br i1 %exitcond, label %inner.loop.end, label %inner.for.body

inner.loop.end:                               ; preds = %inner.for.inc
  store i32 %1, ptr %0, align 4
  br label %outer.loop.end

outer.loop.end:
  %inc = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %inc, 100
  br i1 %cmp, label %exit, label %outer.loop.for.body

exit:
  ret void
}