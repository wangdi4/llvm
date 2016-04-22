; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Src code-
;  for (i = 0; i < NN; ++i)
;   for (j = 0; j < (NN-1)-i; ++j)
;    R2[j][i] = S2[j][i];


; Check parsing output for the loop verifying that '%inc.lcssa19 = smax(1, %0)' is parsed correctly by reverse engineering %0.

; CHECK:      + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK-NEXT: |   %inc.lcssa19 = 0;
; CHECK-NEXT: |   if (i1 < 31)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %0 = 31  -  i1;
; CHECK-NEXT: |      + DO i2 = 0, smax(1, %0) + -1, 1   <DO_LOOP>
; CHECK-NEXT: |      |   %2 = {al:4}(i32*)(@S2)[0][i2][i1];
; CHECK-NEXT: |      |   {al:4}(i32*)(@R2)[0][i2][i1] = %2;
; CHECK-NEXT: |      + END LOOP
; CHECK-NEXT: |      %inc.lcssa19 = smax(1, %0);
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP



;Module Before HIR; ModuleID = 'v430.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = common global i32 0, align 4
@j = common global i32 0, align 4
@S2 = common global [32 x [32 x float]] zeroinitializer, align 16
@R2 = common global [32 x [32 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @test() #0 {
entry:
  store i32 0, i32* @i, align 4
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.inc10 ]
  %cmp215 = icmp slt i64 %indvars.iv22, 31
  br i1 %cmp215, label %for.body3.lr.ph, label %for.inc10

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %0 = sub nuw nsw i64 31, %indvars.iv22
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @S2, i64 0, i64 %indvars.iv, i64 %indvars.iv22
  %1 = bitcast float* %arrayidx5 to i32*
  %2 = load i32, i32* %1, align 4
  %arrayidx9 = getelementptr inbounds [32 x [32 x float]], [32 x [32 x float]]* @R2, i64 0, i64 %indvars.iv, i64 %indvars.iv22
  %3 = bitcast float* %arrayidx9 to i32*
  store i32 %2, i32* %3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp2 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp2, label %for.body3, label %for.inc10.loopexit

for.inc10.loopexit:                               ; preds = %for.body3
  %4 = trunc i64 %indvars.iv.next to i32
  br label %for.inc10

for.inc10:                                        ; preds = %for.inc10.loopexit, %for.cond1.preheader
  %inc.lcssa19 = phi i32 [ 0, %for.cond1.preheader ], [ %4, %for.inc10.loopexit ]
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond = icmp eq i64 %indvars.iv.next23, 32
  br i1 %exitcond, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc10
  store i32 %inc.lcssa19, i32* @j, align 4
  store i32 32, i32* @i, align 4
  ret i32 undef
}

