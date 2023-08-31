; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the Select instruction wasn't selected as
; candidate because there is a convergent call.

; Before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   @llvm.nvvm.barrier0();
;       |
;       |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>
;       |   |   %tmp1 = (%n == 20) ? 0 : 1;
;       |   |   (%p)[i2] = i2 + %tmp1;
;       |   + END LOOP
;       + END LOOP
; END REGION

; After transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   @llvm.nvvm.barrier0();
; CHECK:       |
; CHECK:       |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>
; CHECK:       |   |   %tmp1 = (%n == 20) ? 0 : 1;
; CHECK:       |   |   (%p)[i2] = i2 + %tmp1;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture %p, i32 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 20
  %j.linear.iv = alloca i32, align 4
  br label %outer.loop.ph

outer.loop.ph:
  br label %outer.loop.for.body

outer.loop.for.body:
  %indvars.iv = phi i64 [ 0, %outer.loop.ph ], [ %inc, %outer.loop.end ]
  tail call void @llvm.nvvm.barrier0()
  %cmp2 = icmp sgt i64 %m, 0
  br i1 %cmp2, label %inner.for.body.lr.ph, label %outer.loop.end

inner.for.body.lr.ph:
  br label %inner.for.body

inner.for.body:                               ; preds = %inner.for.inc, %inner.for.body.lr.ph
  %indvars.iv2 = phi i64 [ %indvars.iv.next, %inner.for.inc ], [ 0, %inner.for.body.lr.ph ]
  %tmp1 = select i1 %cmp1, i32 0, i32 1
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv2
  %tr = trunc i64 %indvars.iv2 to i32
  %a = add i32 %tmp1, %tr
  store i32 %a, ptr %arrayidx, align 4
  br label %inner.for.inc

inner.for.inc:                                ; preds = %inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv2, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %m
  br i1 %exitcond, label %outer.end, label %inner.for.body

outer.end:                               ; preds = %inner.for.inc
  br label %outer.loop.end

outer.loop.end:
  %inc = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %inc, 100
  br i1 %cmp, label %exit, label %outer.loop.for.body

exit:
  ret void
}

declare void @llvm.nvvm.barrier0() #0

attributes #0 = { convergent nounwind }