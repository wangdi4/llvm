; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output < %s 2>&1 | FileCheck %s


; Src code-
;  for (i = 0; i < NN; ++i)
;   for (j = 0; j < (NN-1)-i; ++j)
;    R2[j][i] = S2[j][i];


; Check parsing output for the loop verifying that '%inc.lcssa19 = smax(1, %0)' is parsed correctly by reverse engineering %0.
; NOTE: smax was optimized away due to ZTT recognition.

; CHECK: + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK: |   %inc.lcssa19 = 0;
; CHECK: |
; CHECK: |   + DO i2 = 0, -1 * i1 + 30, 1   <DO_LOOP>  <MAX_TC_EST = 31>  <LEGAL_MAX_TC = 31>
; CHECK: |   |   %2 = (i32*)(@S2)[0][i2][i1];
; CHECK: |   |   (i32*)(@R2)[0][i2][i1] = %2;
; CHECK: |   |   %indvars.iv.next = i2  +  1;
; CHECK: |   + END LOOP
; CHECK: |      %inc.lcssa19 = %indvars.iv.next;
; CHECK: |
; CHECK: + END LOOP



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
  store i32 0, ptr @i, align 4
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
  %arrayidx5 = getelementptr inbounds [32 x [32 x float]], ptr @S2, i64 0, i64 %indvars.iv, i64 %indvars.iv22
  %1 = bitcast ptr %arrayidx5 to ptr
  %2 = load i32, ptr %1, align 4
  %arrayidx9 = getelementptr inbounds [32 x [32 x float]], ptr @R2, i64 0, i64 %indvars.iv, i64 %indvars.iv22
  %3 = bitcast ptr %arrayidx9 to ptr
  store i32 %2, ptr %3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp2 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp2, label %for.body3, label %for.inc10.loopexit

for.inc10.loopexit:                               ; preds = %for.body3
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.body3 ]
  %4 = trunc i64 %indvars.iv.next.lcssa to i32
  br label %for.inc10

for.inc10:                                        ; preds = %for.inc10.loopexit, %for.cond1.preheader
  %inc.lcssa19 = phi i32 [ 0, %for.cond1.preheader ], [ %4, %for.inc10.loopexit ]
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond = icmp eq i64 %indvars.iv.next23, 32
  br i1 %exitcond, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc10
  %inc.lcssa19.lcssa = phi i32 [ %inc.lcssa19, %for.inc10 ]
  store i32 %inc.lcssa19.lcssa, ptr @j, align 4
  store i32 32, ptr @i, align 4
  ret i32 undef
}

