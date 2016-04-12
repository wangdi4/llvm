; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the load of %A is parsed correctly as a 2 dimentional array.
; CHECK: DO i1 = 0, 63, 1
; CHECK-SAME: DO_LOOP
; CHECK-NEXT: %0 = {al:4}(%A)[i1][i1]
; CHECK-NEXT: %sum.09 = %0  +  %sum.09
; CHECK-NEXT: END LOOP


; ModuleID = 'array-param.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define i32 @foo([64 x i32]* nocapture readonly %A) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %add

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.09 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx2 = getelementptr inbounds [64 x i32], [64 x i32]* %A, i64 %indvars.iv, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %0, %sum.09
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

