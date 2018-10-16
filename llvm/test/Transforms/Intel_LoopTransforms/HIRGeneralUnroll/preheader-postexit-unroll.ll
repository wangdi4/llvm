; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-general-unroll,print<hir>" < %s 2>&1 | FileCheck %s

; Check that unrolling of loop with preheader/postexit is handled correctly.

; CHECK: Function

; CHECK: |      %fs.promoted = (%fs)[0];
; CHECK: |      %add21136 = %fs.promoted;
; CHECK: |   + DO i2 = 0, zext.i32.i64((49 + (-1 * trunc.i64.i32(%indvars.iv169)))), 1   <DO_LOOP>  <MAX_TC_EST = 42>
; CHECK: |   |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |   |   (%ti)[0][-1 * i1 + i2 + 58][-1 * i1 + i2 + 58] = %4;
; CHECK: |   |   %5 = (%n)[0][-1 * i1 + i2 + 58];
; CHECK: |   |   %add21136 = -1 * i1 + i2 + %5 + 58  +  %add21136;
; CHECK: |   + END LOOP
; CHECK: |      (%fs)[0] = %add21136;

; CHECK: Function

; CHECK: |      %fs.promoted = (%fs)[0];
; CHECK: |      %add21136 = %fs.promoted;
; CHECK: |      %tgu = (zext.i32.i64((49 + (-1 * trunc.i64.i32(%indvars.iv169)))) + 1)/u4;
; CHECK: |      + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |      |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |      |   (%ti)[0][-1 * i1 + 4 * i2 + 58][-1 * i1 + 4 * i2 + 58] = %4;
; CHECK: |      |   %5 = (%n)[0][-1 * i1 + 4 * i2 + 58];
; CHECK: |      |   %add21136 = -1 * i1 + 4 * i2 + %5 + 58  +  %add21136;
; CHECK: |      |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |      |   (%ti)[0][-1 * i1 + 4 * i2 + 59][-1 * i1 + 4 * i2 + 59] = %4;
; CHECK: |      |   %5 = (%n)[0][-1 * i1 + 4 * i2 + 59];
; CHECK: |      |   %add21136 = -1 * i1 + 4 * i2 + %5 + 59  +  %add21136;
; CHECK: |      |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |      |   (%ti)[0][-1 * i1 + 4 * i2 + 60][-1 * i1 + 4 * i2 + 60] = %4;
; CHECK: |      |   %5 = (%n)[0][-1 * i1 + 4 * i2 + 60];
; CHECK: |      |   %add21136 = -1 * i1 + 4 * i2 + %5 + 60  +  %add21136;
; CHECK: |      |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |      |   (%ti)[0][-1 * i1 + 4 * i2 + 61][-1 * i1 + 4 * i2 + 61] = %4;
; CHECK: |      |   %5 = (%n)[0][-1 * i1 + 4 * i2 + 61];
; CHECK: |      |   %add21136 = -1 * i1 + 4 * i2 + %5 + 61  +  %add21136;
; CHECK: |      + END LOOP
; CHECK: |      + DO i2 = 4 * %tgu, zext.i32.i64((49 + (-1 * trunc.i64.i32(%indvars.iv169)))), 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |      |   %4 = (%ti)[0][-1 * i1 + 58][-1 * i1 + 57];
; CHECK: |      |   (%ti)[0][-1 * i1 + i2 + 58][-1 * i1 + i2 + 58] = %4;
; CHECK: |      |   %5 = (%n)[0][-1 * i1 + i2 + 58];
; CHECK: |      |   %add21136 = -1 * i1 + i2 + %5 + 58  +  %add21136;
; CHECK: |      + END LOOP
; CHECK: |      (%fs)[0] = %add21136;


; ModuleID = 't47.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() {
entry:
  %k = alloca i32, align 4
  %i = alloca i32, align 4
  %fs = alloca i32, align 4
  %k7 = alloca i32, align 4
  %n1 = alloca i32, align 4
  %ti = alloca [100 x [100 x i32]], align 16
  %a = alloca [100 x i32], align 16
  %n = alloca [100 x i32], align 16
  %ep = alloca [100 x [100 x i32]], align 16
  %s7 = alloca [100 x i32], align 16
  %x = alloca [100 x i32], align 16
  store i32 69, i32* %k, align 4
  store i32 74, i32* %i, align 4
  store i32 2, i32* %fs, align 4
  store i32 30, i32* %k7, align 4
  store i32 74, i32* %n1, align 4
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.48, %entry
  %indvars.iv169 = phi i64 [ 58, %entry ], [ %7, %for.inc.48 ]
  %indvars.iv165 = phi i32 [ 53, %entry ], [ %indvars.iv.next166, %for.inc.48 ]
  %indvars.iv163 = phi i32 [ -54, %entry ], [ %indvars.iv.next164, %for.inc.48 ]
  %0 = phi i32 [ 58, %entry ], [ %dec49, %for.inc.48 ]
  %1 = zext i32 %indvars.iv163 to i64
  %2 = add i64 %indvars.iv169, %1
  %cmp8.128 = icmp ult i32 %0, 50
  br i1 %cmp8.128, label %for.body.9.lr.ph, label %for.inc.48

for.body.9.lr.ph:                                 ; preds = %for.cond.7.preheader
  %fs.promoted = load i32, i32* %fs, align 4
  %3 = add nsw i64 %indvars.iv169, -1
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ti, i64 0, i64 %indvars.iv169, i64 %3
  br label %for.body.9

for.body.9:                                       ; preds = %for.body.9, %for.body.9.lr.ph
  %indvars.iv148 = phi i64 [ %indvars.iv169, %for.body.9.lr.ph ], [ %indvars.iv.next149, %for.body.9 ]
  %add21136 = phi i32 [ %fs.promoted, %for.body.9.lr.ph ], [ %add21, %for.body.9 ]
  %4 = load i32, i32* %arrayidx12, align 4
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %ti, i64 0, i64 %indvars.iv148, i64 %indvars.iv148
  store i32 %4, i32* %arrayidx16, align 4
  %arrayidx20 = getelementptr inbounds [100 x i32], [100 x i32]* %n, i64 0, i64 %indvars.iv148
  %5 = load i32, i32* %arrayidx20, align 4
  %6 = trunc i64 %indvars.iv148 to i32
  %add = add i32 %6, %5
  %add21 = add i32 %add, %add21136
  %indvars.iv.next149 = add nuw nsw i64 %indvars.iv148, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next149 to i32
  %exitcond154 = icmp eq i32 %lftr.wideiv, 50
  br i1 %exitcond154, label %for.cond.7.for.cond.22.preheader_crit_edge, label %for.body.9

for.cond.7.for.cond.22.preheader_crit_edge:       ; preds = %for.body.9
  %add21.lcssa = phi i32 [ %add21, %for.body.9 ]
  store i32 %add21.lcssa, i32* %fs, align 4
  %arrayidx18.le = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %2
  br label %for.inc.48

for.inc.48:                                       ; preds = %for.cond.22.preheader, %for.cond.22.for.inc.48_crit_edge
  %7 = add nsw i64 %indvars.iv169, -1
  %dec49 = add nsw i32 %0, -1
  %indvars.iv.next164 = add nsw i32 %indvars.iv163, 1
  %indvars.iv.next166 = add nsw i32 %indvars.iv165, -1
  %exitcond173 = icmp eq i32 %indvars.iv.next164, 3
  br i1 %exitcond173, label %for.end.50, label %for.cond.7.preheader

for.end.50:
  ret i32 0
}

