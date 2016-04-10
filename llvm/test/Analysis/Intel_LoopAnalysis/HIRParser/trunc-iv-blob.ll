; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loopnest verifying that the truncated IV of outer loop is reverse engineered correctly as a blob in inner loop upper.
; CHECK: DO i1 = 0, 3
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((-2 + trunc.i64.i32(%indvars.iv)))
; CHECK-NEXT: %1 = {al:4}(%A)[i1 + -1 * i2 + 1]
; CHECK-NEXT: %2 = {al:4}(%A)[i1 + -1 * i2]
; CHECK-NEXT: {al:4}(%A)[i1 + -1 * i2] = -1 * %1 + %2
; CHECK-NEXT: END LOOP
; CHECK-NEXT: %indvars.iv = i1  +  1
; CHECK-NEXT: END LOOP


; ModuleID = 'tt.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc.6
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc.6 ]
  %j.03 = phi i32 [ 0, %entry ], [ %inc, %for.inc.6 ]
  %cmp2.1 = icmp ugt i32 %j.03, 1
  br i1 %cmp2.1, label %for.body.3.lr.ph, label %for.end

for.body.3.lr.ph:                                 ; preds = %for.body
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.lr.ph, %for.inc
  %indvars.iv4 = phi i64 [ %indvars.iv, %for.body.3.lr.ph ], [ %indvars.iv.next5, %for.inc ]
  %0 = trunc i64 %indvars.iv4 to i32
  %add = add i32 %0, 1
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  %1 = load i32, i32* %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv4
  %2 = load i32, i32* %arrayidx5, align 4
  %sub = sub nsw i32 %2, %1
  store i32 %sub, i32* %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.3
  %3 = trunc i64 %indvars.iv4 to i32
  %dec = add i32 %3, -1
  %cmp2 = icmp ugt i32 %dec, 1
  %indvars.iv.next5 = add nsw i64 %indvars.iv4, -1
  br i1 %cmp2, label %for.body.3, label %for.cond.1.for.end_crit_edge

for.cond.1.for.end_crit_edge:                     ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.1.for.end_crit_edge, %for.body
  br label %for.inc.6

for.inc.6:                                        ; preds = %for.end
  %inc = add nuw nsw i32 %j.03, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.body, label %for.end.7

for.end.7:                                        ; preds = %for.inc.6
  ret void
}

