; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the double dereference is parsed correctly.
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %n))
; CHECK-NEXT: %0 = {al:8}(%A)[i1]
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((-1 + %n))
; CHECK-NEXT: %1 = {al:4}(%0)[i2]
; CHECK-NEXT: {al:4}(%0)[i2] = i2 + %1
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP


; ModuleID = 'ptr-load.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32** nocapture readonly %A, i32 %n) {
entry:
  %cmp.20 = icmp sgt i32 %n, 0
  br i1 %cmp.20, label %for.body.3.lr.ph.preheader, label %for.end.8

for.body.3.lr.ph.preheader:                       ; preds = %entry
  br label %for.body.3.lr.ph

for.body.3.lr.ph:                                 ; preds = %for.body.3.lr.ph.preheader, %for.end
  %indvars.iv22 = phi i64 [ %indvars.iv.next23, %for.end ], [ 0, %for.body.3.lr.ph.preheader ]
  %arrayidx = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv22
  %0 = load i32*, i32** %arrayidx, align 8
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.body.3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.3.lr.ph ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %2 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body.3

for.end:                                          ; preds = %for.body.3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %lftr.wideiv24 = trunc i64 %indvars.iv.next23 to i32
  %exitcond25 = icmp eq i32 %lftr.wideiv24, %n
  br i1 %exitcond25, label %for.end.8.loopexit, label %for.body.3.lr.ph

for.end.8.loopexit:                               ; preds = %for.end
  br label %for.end.8

for.end.8:                                        ; preds = %for.end.8.loopexit, %entry
  ret void
}
