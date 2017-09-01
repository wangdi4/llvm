; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -hir-cost-model-throttling=0 -S | FileCheck %s

; Verify that the generated call has the same calling convention and tail marker as the original one.

; CHECK: region.0:
; CHECK: tail call fastcc i32 @bar

; ModuleID = 'call-conv.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define internal fastcc i32 @bar(i32 %i) {
entry:
  %mul = mul nsw i32 %i, %i
  ret i32 %mul
}

; Function Attrs: nounwind uwtable
define void @foo(i32* %A, i32 %n) #0 {
entry:
  %cmp.1 = icmp sgt i32 %n, 0
  br i1 %cmp.1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %call = tail call fastcc i32 @bar(i32 %i.02)
  %idxprom = sext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %call, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

