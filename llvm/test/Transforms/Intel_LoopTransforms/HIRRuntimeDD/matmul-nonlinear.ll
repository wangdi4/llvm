; Test sorting in ddref gathering where a ref occurs twice but with different def level
;
; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd" -aa-pipeline="basic-aa" -debug-only=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; void
; FMX2Multiply(float **A, float **B, float **C, int m, int p, int n)
; {
;   int i, j, k;
;
;   for (i = 0; i < m; i++)
;     for (j = 0; j < n; j++)
;       {
;         C[i][j] = 0.;
;         for (k = 0; k < p; k++)
;           C[i][j] += A[i][p] * B[p][j];
;       }
; }
;
;*** IR Dump Before HIR RuntimeDD Multiversioning (hir-runtime-dd) ***
;Function: FMX2Multiply
;
;<0>          BEGIN REGION { }
;<60>               + DO i1 = 0, sext.i32.i64((-1 + %m)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<7>                |      %0 = (%C)[i1];
;<61>               |   + DO i2 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<13>               |   |   (%0)[i2] = 0.000000e+00;
;<62>               |   |
;<18>               |   |      %1 = (%A)[i1];
;<20>               |   |      %2 = (%B)[%p];
;<22>               |   |      %3 = (%C)[i1];
;<24>               |   |      %.pre = (%3)[i2];
;<25>               |   |      %4 = %.pre;
;<62>               |   |   + DO i3 = 0, %p + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<29>               |   |   |   %5 = (%1)[%p];
;<30>               |   |   |   %6 = (%2)[i2];
;<31>               |   |   |   %mul = %5  *  %6;
;<32>               |   |   |   %4 = %mul  +  %4;
;<33>               |   |   |   (%3)[i2] = %4;
;<62>               |   |   + END LOOP
;<61>               |   + END LOOP
;<60>               + END LOOP
;<0>          END REGION
;
; CHECK: LOOPOPT_OPTREPORT: [RTDD] Loop {{[0-9]+}}: OK
;
; ModuleID = 'matmul-nonlinear.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @FMX2Multiply(ptr nocapture readonly %A, ptr nocapture readonly %B, ptr nocapture readonly %C, i32 %m, i32 %p, i32 %n) {
entry:
  %cmp46 = icmp sgt i32 %m, 0
  br i1 %cmp46, label %for.cond1.preheader.lr.ph, label %for.end26

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp244 = icmp sgt i32 %n, 0
  %cmp742 = icmp sgt i32 %p, 0
  %idxprom9 = sext i32 %p to i64
  %arrayidx15 = getelementptr inbounds ptr, ptr %B, i64 %idxprom9
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc24, %for.cond1.preheader.lr.ph
  %indvars.iv49 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next50, %for.inc24 ]
  br i1 %cmp244, label %for.body3.lr.ph, label %for.inc24

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %arrayidx = getelementptr inbounds ptr, ptr %C, i64 %indvars.iv49
  %0 = load ptr, ptr %arrayidx, align 8
  %arrayidx11 = getelementptr inbounds ptr, ptr %A, i64 %indvars.iv49
  br label %for.body3

for.body3:                                        ; preds = %for.inc21, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc21 ]
  %arrayidx5 = getelementptr inbounds float, ptr %0, i64 %indvars.iv
  store float 0.000000e+00, ptr %arrayidx5, align 4
  br i1 %cmp742, label %for.body8.lr.ph, label %for.inc21

for.body8.lr.ph:                                  ; preds = %for.body3
  %1 = load ptr, ptr %arrayidx11, align 8
  %arrayidx12 = getelementptr inbounds float, ptr %1, i64 %idxprom9
  %2 = load ptr, ptr %arrayidx15, align 8
  %arrayidx16 = getelementptr inbounds float, ptr %2, i64 %indvars.iv
  %3 = load ptr, ptr %arrayidx, align 8
  %arrayidx20 = getelementptr inbounds float, ptr %3, i64 %indvars.iv
  %.pre = load float, ptr %arrayidx20, align 4
  br label %for.body8

for.body8:                                        ; preds = %for.body8, %for.body8.lr.ph
  %4 = phi float [ %.pre, %for.body8.lr.ph ], [ %add, %for.body8 ]
  %k.043 = phi i32 [ 0, %for.body8.lr.ph ], [ %inc, %for.body8 ]
  %5 = load float, ptr %arrayidx12, align 4
  %6 = load float, ptr %arrayidx16, align 4
  %mul = fmul float %5, %6
  %add = fadd float %mul, %4
  store float %add, ptr %arrayidx20, align 4
  %inc = add nuw nsw i32 %k.043, 1
  %exitcond = icmp eq i32 %inc, %p
  br i1 %exitcond, label %for.inc21.loopexit, label %for.body8

for.inc21.loopexit:                               ; preds = %for.body8
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21.loopexit, %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond48 = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond48, label %for.inc24.loopexit, label %for.body3

for.inc24.loopexit:                               ; preds = %for.inc21
  br label %for.inc24

for.inc24:                                        ; preds = %for.inc24.loopexit, %for.cond1.preheader
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %lftr.wideiv51 = trunc i64 %indvars.iv.next50 to i32
  %exitcond52 = icmp eq i32 %lftr.wideiv51, %m
  br i1 %exitcond52, label %for.end26.loopexit, label %for.cond1.preheader

for.end26.loopexit:                               ; preds = %for.inc24
  br label %for.end26

for.end26:                                        ; preds = %for.end26.loopexit, %entry
  ret void
}
