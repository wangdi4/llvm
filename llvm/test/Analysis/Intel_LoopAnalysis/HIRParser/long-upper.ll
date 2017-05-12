; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the i64 type upper is truncated to i32.
; CHECK: DO i1 = 0, trunc.i64.i32((-1 + %n))
; CHECK-NEXT: (%A)[i1] = i1
; CHECK-NEXT: END LOOP



; ModuleID = 'long_upper.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %A, i64 %n) {
entry:
  %cmp.7 = icmp sgt i64 %n, 0
  br i1 %cmp.7, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %conv9 = phi i64 [ 0, %for.body.lr.ph ], [ %conv, %for.body ]
  %i.08 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %conv9
  store i64 %conv9, i64* %arrayidx, align 8
  %inc = add nsw i32 %i.08, 1
  %conv = sext i32 %inc to i64
  %cmp = icmp slt i64 %conv, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

