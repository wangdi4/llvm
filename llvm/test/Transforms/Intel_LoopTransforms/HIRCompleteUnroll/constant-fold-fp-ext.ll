; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -disable-output -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that fpext and fptrunc casts are simplified by constant propagation
; after complete unroll.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   %sum1.018 = 0.000000e+00;
; CHECK: |
; CHECK: |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK: |   |   %fpext = fpext.float.double(%sum1.018);
; CHECK: |   |   %add = (@_ZL4glob)[0][i2]  +  %fpext;
; CHECK: |   |   %sum1.018 = fptrunc.double.float(%add);
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %sum.021 = %add  +  %sum.021;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   %sum.021 = 7.000000e+00  +  %sum.021;
; CHECK: + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL4glob = internal unnamed_addr constant [3 x double] [double 1.000000e+00, double 2.000000e+00, double 4.000000e+00], align 16

define dso_local double @_Z3fooi(i32 %n) local_unnamed_addr {
entry:
  %cmp20 = icmp sgt i32 %n, 0
  br i1 %cmp20, label %for.cond1.preheader.preheader, label %for.cond.cleanup

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.cond.cleanup3
  %ii.022 = phi i32 [ %inc7, %for.cond.cleanup3 ], [ 0, %for.cond1.preheader.preheader ]
  %sum.021 = phi double [ %add5, %for.cond.cleanup3 ], [ 0.000000e+00, %for.cond1.preheader.preheader ]
  br label %for.body4

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %sum1.018 = phi float [ 0.000000e+00, %for.cond1.preheader ], [ %fptrunc, %for.body4 ]
  %arrayidx = getelementptr inbounds [3 x double], [3 x double]* @_ZL4glob, i64 0, i64 %indvars.iv
  %0 = load double, double* %arrayidx, align 8
  %fpext = fpext float %sum1.018 to double
  %add = fadd double %0, %fpext
  %fptrunc = fptrunc double %add to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi double [ %add, %for.body4 ]
  %add5 = fadd double %add.lcssa, %sum.021
  %inc7 = add nuw nsw i32 %ii.022, 1
  %exitcond23 = icmp eq i32 %inc7, %n
  br i1 %exitcond23, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  %add5.lcssa = phi double [ %add5, %for.cond.cleanup3 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %add5.lcssa, %for.cond.cleanup.loopexit ]
  ret double %sum.0.lcssa
}

