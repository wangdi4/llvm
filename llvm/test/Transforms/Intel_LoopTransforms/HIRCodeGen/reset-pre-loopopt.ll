; RUN: opt -hir-ssa-deconstruction -hir-cg -S %s | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S %s | FileCheck %s

; Verify that CG removes "pre_loopopt" on the function irrespective of whether
; we generate code for it or not.

; CHECK-NOT: pre_loopopt

source_filename = "div2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %A, i64 %n, i64 %m) #0 {
entry:
  %div = sdiv i64 %n, 2
  %cmp6 = icmp sgt i64 %div, 0
  br i1 %cmp6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %j.07 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %conv = trunc i64 %j.07 to i32
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %j.07
  store i32 %conv, i32* %arrayidx, align 4
  %inc = add nuw nsw i64 %j.07, 1
  %exitcond = icmp eq i64 %inc, %div
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { "pre_loopopt" }

