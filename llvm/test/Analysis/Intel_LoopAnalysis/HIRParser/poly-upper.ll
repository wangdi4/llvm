; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the polynomial upper of inenr loop is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %indvars.iv23.out = %indvars.iv23
; CHECK-NEXT: %k.020 = %k.020  +  i1
; CHECK-NEXT: if (%k.020 > 0)
; CHECK-NEXT: {
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((-1 + %indvars.iv23.out)),
; CHECK-NEXT: {al:4}(%A)[i2] = i2;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: }
; CHECK-NEXT: %indvars.iv23 = %indvars.iv23  +  i1 + 1
; CHECK-NEXT: END LOOP


; ModuleID = 'poly-upper.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %A, i32 %n) {
entry:
  %cmp.18 = icmp sgt i32 %n, 0
  br i1 %cmp.18, label %for.body.preheader, label %for.end.6

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc.4
  %indvars.iv23 = phi i32 [ %indvars.iv.next24, %for.inc.4 ], [ 0, %for.body.preheader ]
  %indvars.iv21 = phi i32 [ %indvars.iv.next22, %for.inc.4 ], [ 1, %for.body.preheader ]
  %k.020 = phi i32 [ %add, %for.inc.4 ], [ 0, %for.body.preheader ]
  %i.019 = phi i32 [ %inc5, %for.inc.4 ], [ 0, %for.body.preheader ]
  %add = add nsw i32 %k.020, %i.019
  %cmp2.16 = icmp sgt i32 %add, 0
  br i1 %cmp2.16, label %for.body.3.preheader, label %for.inc.4

for.body.3.preheader:                             ; preds = %for.body
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.preheader, %for.body.3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body.3 ], [ 0, %for.body.3.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %indvars.iv23
  br i1 %exitcond, label %for.inc.4.loopexit, label %for.body.3

for.inc.4.loopexit:                               ; preds = %for.body.3
  br label %for.inc.4

for.inc.4:                                        ; preds = %for.inc.4.loopexit, %for.body
  %inc5 = add nuw nsw i32 %i.019, 1
  %indvars.iv.next22 = add nuw i32 %indvars.iv21, 1
  %indvars.iv.next24 = add i32 %indvars.iv23, %indvars.iv21
  %exitcond25 = icmp eq i32 %inc5, %n
  br i1 %exitcond25, label %for.end.6.loopexit, label %for.body

for.end.6.loopexit:                               ; preds = %for.inc.4
  br label %for.end.6

for.end.6:                                        ; preds = %for.end.6.loopexit, %entry
  ret void
}
