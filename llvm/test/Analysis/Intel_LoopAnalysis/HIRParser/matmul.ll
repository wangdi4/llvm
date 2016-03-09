; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the single operand phi(%add.lcssa) is optimized out.

; CHECK: DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK-NEXT: DO i2 = 0, %N + -1, 1   <DO_LOOP>
; CHECK-NEXT: %arrayidx7.promoted = {al:8}(@c)[0][i1][i2];
; CHECK-NEXT: %0 = %arrayidx7.promoted;
; CHECK-NEXT: DO i3 = 0, %N + -1, 1   <DO_LOOP>
; CHECK-NEXT: %1 = {al:8}(@a)[0][i1][i3];
; CHECK-NEXT: %2 = {al:8}(@b)[0][i3][i2];
; CHECK-NEXT: %mul = %1  *  %2;
; CHECK-NEXT: %0 = %0  +  %mul;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: {al:8}(@c)[0][i1][i2] = %0;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP


; ModuleID = 'matmul.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common global [1024 x [1024 x double]] zeroinitializer, align 16
@a = common global [1024 x [1024 x double]] zeroinitializer, align 16
@b = common global [1024 x [1024 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @sub(i64 %N) {
entry:
  %cmp.41 = icmp sgt i64 %N, 0
  br i1 %cmp.41, label %for.cond.4.preheader.preheader.preheader, label %for.end.19

for.cond.4.preheader.preheader.preheader:         ; preds = %entry
  br label %for.cond.4.preheader.preheader

for.cond.4.preheader.preheader:                   ; preds = %for.cond.4.preheader.preheader.preheader, %for.inc.17
  %i.042 = phi i64 [ %inc18, %for.inc.17 ], [ 0, %for.cond.4.preheader.preheader.preheader ]
  br label %for.body.6.lr.ph

for.body.6.lr.ph:                                 ; preds = %for.cond.4.preheader.preheader, %for.inc.14
  %j.039 = phi i64 [ %inc15, %for.inc.14 ], [ 0, %for.cond.4.preheader.preheader ]
  %arrayidx7 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @c, i64 0, i64 %i.042, i64 %j.039
  %arrayidx7.promoted = load double, double* %arrayidx7, align 8
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.body.6.lr.ph
  %0 = phi double [ %arrayidx7.promoted, %for.body.6.lr.ph ], [ %add, %for.body.6 ]
  %k.037 = phi i64 [ 0, %for.body.6.lr.ph ], [ %inc, %for.body.6 ]
  %arrayidx9 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @a, i64 0, i64 %i.042, i64 %k.037
  %1 = load double, double* %arrayidx9, align 8
  %arrayidx11 = getelementptr inbounds [1024 x [1024 x double]], [1024 x [1024 x double]]* @b, i64 0, i64 %k.037, i64 %j.039
  %2 = load double, double* %arrayidx11, align 8
  %mul = fmul double %1, %2
  %add = fadd double %0, %mul
  %inc = add nuw nsw i64 %k.037, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.inc.14, label %for.body.6

for.inc.14:                                       ; preds = %for.body.6
  %add.lcssa = phi double [ %add, %for.body.6 ]
  store double %add.lcssa, double* %arrayidx7, align 8
  %inc15 = add nuw nsw i64 %j.039, 1
  %exitcond44 = icmp eq i64 %inc15, %N
  br i1 %exitcond44, label %for.inc.17, label %for.body.6.lr.ph

for.inc.17:                                       ; preds = %for.inc.14
  %inc18 = add nuw nsw i64 %i.042, 1
  %exitcond45 = icmp eq i64 %inc18, %N
  br i1 %exitcond45, label %for.end.19.loopexit, label %for.cond.4.preheader.preheader

for.end.19.loopexit:                              ; preds = %for.inc.17
  br label %for.end.19

for.end.19:                                       ; preds = %for.end.19.loopexit, %entry
  ret i32 0
}

