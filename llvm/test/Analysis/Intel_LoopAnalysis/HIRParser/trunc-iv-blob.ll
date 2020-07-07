; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check parsing output for the loopnest verifying that the truncated IV of outer loop is reverse engineered correctly as a blob in inner loop upper.
; CHECK: + DO i1 = 0, %ub + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, zext.i32.i64((-2 + trunc.i64.i32(%indvars.iv))), 1   <DO_LOOP>
; CHECK: |   |   %t1 = (%A)[i1 + -1 * i2 + 1];
; CHECK: |   |   (%A)[i1 + -1 * i2] = %t1
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %indvars.iv = i1 + 1;
; CHECK: + END LOOP


; ModuleID = 'tt.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %A, i64 %ub) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc.6
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc.6 ]
  %t3 = trunc i64 %indvars.iv to i32
  %cmp2.1 = icmp ugt i32 %t3, 1
  br i1 %cmp2.1, label %for.body.3.lr.ph, label %for.end

for.body.3.lr.ph:                                 ; preds = %for.body
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.lr.ph, %for.body.3
  %indvars.iv4 = phi i64 [ %indvars.iv, %for.body.3.lr.ph ], [ %indvars.iv.next5, %for.body.3 ]
  %add = add nsw i64 %indvars.iv4, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %add
  %t1 = load i32, i32* %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv4
  store i32 %t1, i32* %arrayidx5, align 4
  %indvars.iv.next5 = add nsw i64 %indvars.iv4, -1
  %tr.iv = trunc i64 %indvars.iv.next5 to i32
  %cmp2 = icmp ugt i32 %tr.iv, 1
  br i1 %cmp2, label %for.body.3, label %for.cond.1.for.end_crit_edge

for.cond.1.for.end_crit_edge:                     ; preds = %for.body.3
  br label %for.end

for.end:                                          ; preds = %for.cond.1.for.end_crit_edge, %for.body
  br label %for.inc.6

for.inc.6:                                        ; preds = %for.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, %ub
  br i1 %exitcond, label %for.body, label %for.end.7

for.end.7:                                        ; preds = %for.inc.6
  ret void
}

