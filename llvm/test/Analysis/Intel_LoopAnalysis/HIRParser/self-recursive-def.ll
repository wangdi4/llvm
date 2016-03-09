; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loopnest verifying that there is not self-recursive definition.
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %n))
; CHECK-NEXT: %.pre = {al:4}(%A)[i1]
; CHECK-NEXT: %0 = %.pre
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((-1 + %n))
; CHECK-NEXT: %1 = {al:4}(%B)[i2]
; CHECK-NEXT: %0 = %0  +  %1
; CHECK-NEXT: {al:4}(%A)[i1] = %0
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP

; Check that every instance of %0 is parsed as a non-linear self blob.
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck -check-prefix=DETAIL %s
; DETAIL: %0 = %0  +  %1
; DETAIL: NON-LINEAR i32 %0
; DETAIL: NON-LINEAR i32 %0
; DETAIL: {al:4}(%A)[i1] = %0
; DETAIL: NON-LINEAR i32 %0


; ModuleID = 'self-recursive-def.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %A, i32* nocapture readonly %B, i32 %n) {
entry:
  %cmp.18 = icmp sgt i32 %n, 0
  br i1 %cmp.18, label %for.body.3.lr.ph.preheader, label %for.end.8

for.body.3.lr.ph.preheader:                       ; preds = %entry
  br label %for.body.3.lr.ph

for.body.3.lr.ph:                                 ; preds = %for.body.3.lr.ph.preheader, %for.inc.6
  %indvars.iv20 = phi i64 [ %indvars.iv.next21, %for.inc.6 ], [ 0, %for.body.3.lr.ph.preheader ]
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv20
  %.pre = load i32, i32* %arrayidx5, align 4
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %0 = phi i32 [ %.pre, %for.body.3.lr.ph ], [ %add, %for.body.3 ]
  %indvars.iv = phi i64 [ 0, %for.body.3.lr.ph ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc.6, label %for.body.3

for.inc.6:                                        ; preds = %for.body.3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %lftr.wideiv22 = trunc i64 %indvars.iv.next21 to i32
  %exitcond23 = icmp eq i32 %lftr.wideiv22, %n
  br i1 %exitcond23, label %for.end.8.loopexit, label %for.body.3.lr.ph

for.end.8.loopexit:                               ; preds = %for.inc.6
  br label %for.end.8

for.end.8:                                        ; preds = %for.end.8.loopexit, %entry
  ret void
}
