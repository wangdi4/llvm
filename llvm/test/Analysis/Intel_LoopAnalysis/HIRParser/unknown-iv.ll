; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop

; CHECK: + DO i1 = 0, 8, 1   <DO_LOOP>
; CHECK: |   (%A)[%i.05] = %i.05;
; CHECK: |   %i.05 = %i.05  /  2;
; CHECK: + END LOOP


; ModuleID = 'unknown-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i32 [ 1001, %entry ], [ %div, %for.body ]
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.05, i32* %arrayidx, align 4 
  %div = sdiv i32 %i.05, 2
  %cmp = icmp sgt i32 %i.05, 3
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

