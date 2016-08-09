; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we form SCC containing single operand phi (%sum.025 -> %sum.121 -> %add -> %add.lcssa) resulting in perfect loopnest.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   |   %0 = (@A)[0][i2][i1];
; CHECK: |   |   %sum.025 = %sum.025  +  %0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; ModuleID = 'interchange10.c'
source_filename = "interchange10.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x double]] zeroinitializer, align 16
@Gsum = common global double 0.000000e+00, align 8

define void @sub1(i64 %n, i64 %m) {
entry:
  %cmp22 = icmp sgt i64 %n, 0
  br i1 %cmp22, label %for.body3.preheader.preheader, label %for.end7

for.body3.preheader.preheader:                    ; preds = %entry
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc5, %for.body3.preheader.preheader
  %sum.025 = phi double [ %add.lcssa, %for.inc5 ], [ 0.000000e+00, %for.body3.preheader.preheader ]
  %i1.023 = phi i64 [ %inc6, %for.inc5 ], [ 0, %for.body3.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %sum.121 = phi double [ %add, %for.body3 ], [ %sum.025, %for.body3.preheader ]
  %i2.020 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x double]], [1000 x [1000 x double]]* @A, i64 0, i64 %i2.020, i64 %i1.023
  %0 = load double, double* %arrayidx4, align 8
  %add = fadd double %sum.121, %0
  %inc = add nuw nsw i64 %i2.020, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %add.lcssa = phi double [ %add, %for.body3 ]
  %inc6 = add nuw nsw i64 %i1.023, 1
  %exitcond27 = icmp eq i64 %inc6, %n
  br i1 %exitcond27, label %for.end7.loopexit, label %for.body3.preheader

for.end7.loopexit:                                ; preds = %for.inc5
  %add.lcssa.lcssa = phi double [ %add.lcssa, %for.inc5 ]
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add.lcssa.lcssa, %for.end7.loopexit ]
  store double %sum.0.lcssa, double* @Gsum, align 8
  ret void
}
