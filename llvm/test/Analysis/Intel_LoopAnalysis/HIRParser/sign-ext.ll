; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check parsing output for the loop verifying that the subscript is parsed as a sign extended version.
; CHECK: DO i32 i1 = 0, 24
; CHECK-NEXT: {al:4}(%A)[i1 + 7] = i1 + 7
; CHECK-NEXT: (LINEAR i32* %A)[LINEAR sext.i32.i64(i1 + 7)]


; ModuleID = 'loop1.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %i.01 = phi i32 [ 7, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.01, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %exitcond = icmp ne i32 %inc, 32
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret i32 32
}
