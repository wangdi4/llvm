; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll"  -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we deduce that it is profitable to propagate the store to
; (%A)[i2][i3] in the first loop due to the load (%A)[i3][i2] in the second loop
; which has the same shape but different indices.

; HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; |   |   |   (%A)[i2][i3] = 0;
; |   |   + END LOOP
; |   + END LOOP
; |
; |
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; |   |   |   %ld = (%A)[i3][i2];
; |   |   |   (%B)[i3][i2] = %ld + 1;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: Number of memrefs which can be eliminated: 100

; CHECK: + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i3 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   (%A)[i2][i3] = 0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %B, i32 %n) {
entry:
  %A = alloca [10 x i32], align 16
  %cmp30 = icmp sgt i32 %n, 0
  br i1 %cmp30, label %for.body.lr.ph, label %for.end14

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc12
  %i.031 = phi i32 [ 0, %for.body.lr.ph ], [ %inc13, %for.inc12 ]
  br label %for.body3.outer

for.body3.outer:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv.outer = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.outer.next, %for.body3.outer.latch ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.outer ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr %A, i64 %indvars.iv.outer, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.body3.outer.latch, label %for.body3

for.body3.outer.latch:
  %indvars.iv.outer.next = add nuw nsw i64 %indvars.iv.outer, 1
  %exitcond1 = icmp eq i64 %indvars.iv.outer.next, 10
  br i1 %exitcond1, label %for.end, label %for.body3.outer

for.end:                                          ; preds = %for.body3
  br label %for.body6.outer

for.body6.outer:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv6.outer = phi i64 [ 0, %for.end ], [ %indvars.iv6.outer.next, %for.body6.outer.latch ]
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.end
  %indvars.iv6 = phi i64 [ 0, %for.body6.outer ], [ %indvars.iv6.next, %for.body6 ]
  %arrayidxA = getelementptr inbounds [10 x i32], ptr %A, i64 %indvars.iv6, i64 %indvars.iv6.outer
  %ld = load i32, ptr %arrayidxA
  %add = add i32 %ld, 1
  %arrayidxB = getelementptr inbounds [10 x i32], ptr %B, i64 %indvars.iv6, i64 %indvars.iv6.outer
  store i32 %add, ptr %arrayidxB, align 4
  %indvars.iv6.next = add nuw nsw i64 %indvars.iv6, 1
  %exitcond6 = icmp eq i64 %indvars.iv6.next, 10
  br i1 %exitcond6, label %for.body6.outer.latch, label %for.body6

for.body6.outer.latch:
  %indvars.iv6.outer.next = add nuw nsw i64 %indvars.iv6.outer, 1
  %exitcond7 = icmp eq i64 %indvars.iv6.outer.next, 10
  br i1 %exitcond7, label %for.inc12, label %for.body6.outer

for.inc12:                                        ; preds = %for.body6
  %inc13 = add nuw nsw i32 %i.031, 1
  %exitcond35 = icmp eq i32 %inc13, %n
  br i1 %exitcond35, label %for.end14.loopexit, label %for.body3.lr.ph

for.end14.loopexit:                               ; preds = %for.inc12
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %entry
  ret void
}

