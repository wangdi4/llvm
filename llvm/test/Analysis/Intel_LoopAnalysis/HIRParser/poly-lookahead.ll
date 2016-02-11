; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the we are able to handle IV multiplications resulting in polynomial SCEVs by looking ahead.

; CHECK: DO i1 = 0, 69
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((39 + (-1 * trunc.i64.i32(%indvars.iv22))))
; CHECK-NEXT: %0 = i1 + i2 + 3  *  i1 + 3
; CHECK-NEXT: %1 = (%A)[i1 + i2 + 3]
; CHECK-NEXT: %2 = trunc.i64.i32(%0)
; CHECK-NEXT: (%A)[i1 + i2 + 3] = %1 + %2
; CHECK-NEXT: END LOOP
; CHECK-NEXT: %indvars.iv22 = i1 + 4;
; CHECK-NEXT: END LOOP


;Module Before HIR; ModuleID = 'poly-lookahead.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %A) {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.4, %entry
  %indvars.iv22 = phi i64 [ 3, %entry ], [ %indvars.iv.next23, %for.inc.4 ]
  %cmp2.15 = icmp slt i64 %indvars.iv22, 40
  br i1 %cmp2.15, label %for.body.3.preheader, label %for.inc.4

for.body.3.preheader:                             ; preds = %for.cond.1.preheader
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.preheader, %for.body.3
  %indvars.iv19 = phi i64 [ %indvars.iv.next20, %for.body.3 ], [ %indvars.iv22, %for.body.3.preheader ]
  %0 = mul nsw i64 %indvars.iv19, %indvars.iv22
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv19
  %1 = load i32, i32* %arrayidx, align 4
  %2 = trunc i64 %0 to i32
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %arrayidx, align 4
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next20 to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 40
  br i1 %exitcond, label %for.inc.4.loopexit, label %for.body.3

for.inc.4.loopexit:                               ; preds = %for.body.3
  br label %for.inc.4

for.inc.4:                                        ; preds = %for.inc.4.loopexit, %for.cond.1.preheader
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 73
  br i1 %exitcond24, label %for.end.6, label %for.cond.1.preheader

for.end.6:                                        ; preds = %for.inc.4
  ret void
}

