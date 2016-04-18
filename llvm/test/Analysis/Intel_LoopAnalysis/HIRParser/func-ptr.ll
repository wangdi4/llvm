; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that a dereference of function pointer argument is parsed correctly.
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %n))
; CHECK-NEXT: %0 = {al:4}(%A)[0]
; CHECK-NEXT: {al:4}(%B)[i1] = %0
; CHECK-NEXT: END LOOP


; ModuleID = 'func-ptr.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture readonly %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.5 = icmp sgt i32 %n, 0
  br i1 %cmp.5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %0 = load i32, i32* %A, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  store i32 %0, i32* %arrayidx1, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
