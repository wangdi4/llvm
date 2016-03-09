; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output of the loop verifying that the SCEVs with non-generable loop IVs are handled properly.
 
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %0 = {al:4}(%A)[i1];
; CHECK-NEXT: {al:4}(%A)[i1] = 2 * i1 + %0 + ((2 + %k) * %i.04) + 3;
; CHECK-NEXT: END LOOP

; ModuleID = 'outer_non_generable_loop.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %A, i32 %n, i32 %k) {
entry:
  %cmp.3 = icmp sgt i32 %n, 0
  br i1 %cmp.3, label %for.body.lr.ph, label %for.end.12

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.10
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc11, %for.inc.10 ]
  %mul = shl nsw i32 %i.04, 1
  %add = or i32 %mul, 1
  %mul1 = mul nsw i32 %i.04, %k
  %add2 = add nsw i32 %mul1, 2
  %cmp4.1 = icmp sgt i32 %n, 0
  br i1 %cmp4.1, label %for.body.5.lr.ph, label %for.end

for.body.5.lr.ph:                                 ; preds = %for.body
  br label %for.body.5

for.body.5:                                       ; preds = %for.body.5.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body.5.lr.ph ], [ %inc, %for.inc ]
  %add6 = add nsw i32 %add, %j.02
  %add7 = add nsw i32 %add2, %j.02
  %add8 = add nsw i32 %add6, %add7
  %idxprom = sext i32 %j.02 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add9 = add nsw i32 %0, %add8
  store i32 %add9, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.5
  %inc = add nsw i32 %j.02, 1
  %cmp4 = icmp slt i32 %inc, %n
  br i1 %cmp4, label %for.body.5, label %for.cond.3.for.end_crit_edge

for.cond.3.for.end_crit_edge:                     ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.3.for.end_crit_edge, %for.body
  br label %for.inc.10

for.inc.10:                                       ; preds = %for.end
  %inc11 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %add, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end.12_crit_edge

for.cond.for.end.12_crit_edge:                    ; preds = %for.inc.10
  br label %for.end.12

for.end.12:                                       ; preds = %for.cond.for.end.12_crit_edge, %entry
  ret void
}
