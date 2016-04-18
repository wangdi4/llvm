; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the compare instruction is parsed correctly.
; CHECK: DO i1 = 0, ((-1 + sext.i32.i64(%m)) /u %indvars.iv)
; CHECK-NEXT: %3 = {al:4}(%A)[%indvars.iv * i1]
; CHECK-NEXT: {al:4}(%A)[%indvars.iv * i1] = trunc.i64.i32(%indvars.iv) * i1 + %3
; CHECK-NEXT: END LOOP


; ModuleID = 'denom.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32 %m, i32 %n) {
entry:
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 5
  %0 = load i32, i32* %arrayidx, align 4
  %cmp18 = icmp sgt i32 %0, 1
  br i1 %cmp18, label %for.cond1.preheader.lr.ph, label %for.end7

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp216 = icmp sgt i32 %m, 0
  %1 = sext i32 %m to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %for.cond1.preheader.lr.ph
  %2 = phi i32 [ %0, %for.cond1.preheader.lr.ph ], [ %5, %for.inc6 ]
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next, %for.inc6 ]
  %i.019 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %inc, %for.inc6 ]
  br i1 %cmp216, label %for.body3.preheader, label %for.inc6

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv21 = phi i64 [ %indvars.iv.next22, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv21
  %3 = load i32, i32* %arrayidx4, align 4
  %4 = trunc i64 %indvars.iv21 to i32
  %add = add nsw i32 %3, %4
  store i32 %add, i32* %arrayidx4, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, %indvars.iv
  %cmp2 = icmp slt i64 %indvars.iv.next22, %1
  br i1 %cmp2, label %for.body3, label %for.inc6.loopexit

for.inc6.loopexit:                                ; preds = %for.body3
  %.pre = load i32, i32* %arrayidx, align 4
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %5 = phi i32 [ %.pre, %for.inc6.loopexit ], [ %2, %for.cond1.preheader ]
  %inc = add nuw nsw i32 %i.019, 1
  %cmp = icmp slt i32 %inc, %5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp, label %for.cond1.preheader, label %for.end7.loopexit

for.end7.loopexit:                                ; preds = %for.inc6
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}

