; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the IV lievout case is handled properly.

; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %j.0.lcssa = 0
; CHECK-NEXT: DO i2 = 0, %n + -1
; CHECK-NEXT: {al:4}(%A)[i1] = i2
; CHECK-NEXT: END LOOP
; CHECK-NEXT: %j.0.lcssa = %n
; CHECK: %ret.05 = %ret.05  +  %j.0.lcssa
; CHECK-NEXT: END LOOP


; ModuleID = 'iv-loopnest-liveout.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* %A, i32 %n) {
entry:
  %cmp.3 = icmp sgt i32 %n, 0
  br i1 %cmp.3, label %for.body.lr.ph, label %for.end.6

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.4
  %ret.05 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %for.inc.4 ]
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc5, %for.inc.4 ]
  %cmp2.1 = icmp sgt i32 %n, 0
  br i1 %cmp2.1, label %for.body.3.lr.ph, label %for.end

for.body.3.lr.ph:                                 ; preds = %for.body
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body.3.lr.ph ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.04 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %j.02, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.3
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %n
  br i1 %cmp2, label %for.body.3, label %for.cond.1.for.end_crit_edge

for.cond.1.for.end_crit_edge:                     ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.1.for.end_crit_edge, %for.body
  %j.0.lcssa = phi i32 [ %inc, %for.cond.1.for.end_crit_edge ], [ 0, %for.body ]
  %add = add nsw i32 %ret.05, %j.0.lcssa
  br label %for.inc.4

for.inc.4:                                        ; preds = %for.end
  %inc5 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc5, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end.6_crit_edge

for.cond.for.end.6_crit_edge:                     ; preds = %for.inc.4
  br label %for.end.6

for.end.6:                                        ; preds = %for.cond.for.end.6_crit_edge, %entry
  %ret.0.lcssa = phi i32 [ %add, %for.cond.for.end.6_crit_edge ], [ 0, %entry ]
  ret i32 %ret.0.lcssa
}

