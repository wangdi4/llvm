; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output and livein added due to parsing of upper bound
; CHECK: LiveIns:
; CHECK-SAME: %call(%call)
; CHECK: DO i1 = 0
; CHECK-SAME: %call
; CHECK-NEXT: (%A)[i1] = i1
; CHECK-NEXT: END LOOP


; ModuleID = 'upper.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture %A, i32 %n) {
entry:
  %call = tail call i64 @foo1(i32 %n)
  %add = add nsw i64 %call, 2
  %cmp.7 = icmp slt i64 %call, -2
  br i1 %cmp.7, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv, %add
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 undef
}

declare i64 @foo1(i32)
