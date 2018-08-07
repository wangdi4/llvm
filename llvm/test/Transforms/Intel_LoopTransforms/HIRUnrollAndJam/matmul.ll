; RUN: opt < %s -hir-ssa-deconstruction -hir-unroll-and-jam -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we unroll i1 loop by 2 and i2 loop by 8.

; CHECK: Function

; CHECK: + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   + DO i3 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   |   %0 = (@c)[0][i1][i2];
; CHECK: |   |   |   %1 = (@a)[0][i1][i3];
; CHECK: |   |   |   %2 = (@b)[0][i3][i2];
; CHECK: |   |   |   %mul = %1  *  %2;
; CHECK: |   |   |   %add = %0  +  %mul;
; CHECK: |   |   |   (@c)[0][i1][i2] = %add;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: Function

; CHECK: %tgu = (%N)/u4;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 256>
; CHECK: |   %tgu3 = (%N)/u4;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu3 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 256>


; ModuleID = 'matmul.ll'
source_filename = "matmul.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common local_unnamed_addr global [1024 x [1024 x double]] zeroinitializer, align 16

define i32 @sub(i64 %N) local_unnamed_addr {
entry:
  %cmp6 = icmp slt i64 0, %N
  br i1 %cmp6, label %for.cond1.preheader.lr.ph, label %for.end19

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc17
  %i.07 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc18, %for.inc17 ]
  br label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc14
  %j.04 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %inc15, %for.inc14 ]
  br label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %k.02 = phi i64 [ 0, %for.body6.lr.ph ], [ %inc, %for.body6 ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.07, i64 %j.04
  %0 = load double, double* %arrayidx7, align 8
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.07, i64 %k.02
  %1 = load double, double* %arrayidx9, align 8
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.02, i64 %j.04
  %2 = load double, double* %arrayidx11, align 8
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  store double %add, double* %arrayidx7, align 8
  %inc = add nsw i64 %k.02, 1
  %cmp5 = icmp slt i64 %inc, %N
  br i1 %cmp5, label %for.body6, label %for.cond4.for.inc14_crit_edge

for.cond4.for.inc14_crit_edge:                    ; preds = %for.body6
  br label %for.inc14

for.inc14:                                        ; preds = %for.cond4.for.inc14_crit_edge
  %inc15 = add nsw i64 %j.04, 1
  %cmp2 = icmp slt i64 %inc15, %N
  br i1 %cmp2, label %for.cond4.preheader, label %for.cond1.for.inc17_crit_edge

for.cond1.for.inc17_crit_edge:                    ; preds = %for.inc14
  br label %for.inc17

for.inc17:                                        ; preds = %for.cond1.for.inc17_crit_edge
  %inc18 = add nsw i64 %i.07, 1
  %cmp = icmp slt i64 %inc18, %N
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.for.end19_crit_edge

for.cond.for.end19_crit_edge:                     ; preds = %for.inc17
  br label %for.end19

for.end19:                                        ; preds = %for.cond.for.end19_crit_edge, %entry
  ret i32 0
}
