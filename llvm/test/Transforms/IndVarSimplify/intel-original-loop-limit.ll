; RUN: opt -passes="loop(indvars)" < %s -S | FileCheck %s

; Verify that we use the original loop limit %mul16 when changing the loop
; predicate.
; Previously, indvars was generating a new instruction which is same as %mul16
; but without 'nsw' flag.
; This results in loss of information.

; CHECK: icmp ne i32 %inc33, %mul16

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define void @foo(i32* %dest, i32* %src, i32 %div) {
for.cond15.preheader:                             ; preds = %for.cond15.preheader.loopexit, %for.body
  %mul16 = shl nsw i32 %div, 1
  %cmp17100 = icmp sgt i32 %mul16, 0
  br i1 %cmp17100, label %for.body18.lr.ph, label %exit

for.body18.lr.ph:
  br label %for.body18

for.body18:                                       ; preds = %for.body18.lr.ph, %for.body18
  %i.1101 = phi i32 [ 0, %for.body18.lr.ph ], [ %inc33, %for.body18 ]
  %arrayidx21 = getelementptr inbounds i32, i32* %src, i32 1
  %t1 = load i32, i32* %arrayidx21, align 4
  %arrayidx24 = getelementptr inbounds i32, i32* %dest, i32 1
  store i32 %t1, i32* %arrayidx24, align 4
  %inc33 = add nuw nsw i32 %i.1101, 1
  %cmp17 = icmp slt i32 %inc33, %mul16
  br i1 %cmp17, label %for.body18, label %for.cond35.preheader.loopexit

for.cond35.preheader.loopexit:
  br label %exit

exit:
  ret void
}
