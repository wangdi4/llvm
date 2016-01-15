; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the outer loop IV which is the stride for inner loop IV is reverse engineering correctly.
; CHECK: DO i1 = 0, 8190
; CHECK-NEXT: DO i2 = 0, (8191 /u %i.015)
; CHECK-NEXT: %0 = (%A)[%i.015 * i2]
; CHECK-NEXT: (%A)[%i.015 * i2] = %i.015 * i2 + %0
; CHECK-NEXT: END LOOP
; CHECK-NEXT: %i.015 = i1 + 2
; CHECK-NEXT: END LOOP

; ModuleID = 't.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %A) {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.5, %entry
  %i.015 = phi i64 [ 1, %entry ], [ %inc, %for.inc.5 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.cond.1.preheader, %for.body.3
  %j.014 = phi i64 [ 0, %for.cond.1.preheader ], [ %add4, %for.body.3 ]
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %j.014
  %0 = load i64, i64* %arrayidx, align 8
  %add = add i64 %0, %j.014
  store i64 %add, i64* %arrayidx, align 8
  %add4 = add nuw nsw i64 %j.014, %i.015
  %cmp2 = icmp ult i64 %add4, 8192
  br i1 %cmp2, label %for.body.3, label %for.inc.5

for.inc.5:                                        ; preds = %for.body.3
  %inc = add nuw nsw i64 %i.015, 1
  %exitcond = icmp eq i64 %inc, 8192
  br i1 %exitcond, label %for.end.6, label %for.cond.1.preheader

for.end.6:                                        ; preds = %for.inc.5
  ret void
}

