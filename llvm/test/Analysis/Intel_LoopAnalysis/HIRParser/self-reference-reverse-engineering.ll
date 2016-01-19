; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that %0 is not reverse engineered in terms of %1 (the instruction it is an operand of!).
; CHECK: DO i1 = 0, 999
; CHECK-NEXT: %0 = i1  *  i1;
; CHECK-NEXT: %1 = %0  +  1;
; CHECK-NEXT: (%A)[i1] = %1;
; CHECK-NEXT: END LOOP


; ModuleID = 'self-reference-reverse-engineering.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = mul nsw i64 %indvars.iv, %indvars.iv
  %1 = add nuw nsw i64 %0, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %2 = trunc i64 %1 to i32
  store i32 %2, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

