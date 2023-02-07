; RUN: opt -passes="hir-ssa-deconstruction,print<hir-loop-resource>" < %s -disable-output 2>&1 | FileCheck %s

; Verify that we can successfully compute loop resource for loop containing
; call instruction which does not access memory.
; Note that currently calls which do not access memory are assumed to have zero
; cost to work around perf regressions.
; Also note that the loop is classified as 'Branch Bound' which is unintentional
; result in the case where total cost is zero.

; + DO i1 = 0, 4, 1   <DO_LOOP>
; |   %res = @bar();
; + END LOOP


; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:    Integer Operations: 1
; CHECK:    Total Cost: 0
; CHECK:    Branch(Misprediction) Bound
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() {
for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %res = call i32 @bar();
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %res.lcssa = phi i32 [ %res, %for.body ]
  ret void
}

declare i32 @bar() memory(none)
