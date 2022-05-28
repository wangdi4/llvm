; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-mv-const-ub -print-after=hir-mv-const-ub -debug-only=hir-mv-const-ub -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -debug-only=hir-mv-const-ub -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we fail to multiversion loopnest for MAX_TC_EST
; if we cannot compute the integer value for TC blob.

;        BEGIN REGION { }
;             + DO i1 = 0, 999, 1   <DO_LOOP>
;             |   + DO i2 = 0, 2 * %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>
;             |   |   %sub = (%M)[i2]  -  (@A)[0][i1][i2];
;             |   |   (%M)[i2] = %sub;
;             |   + END LOOP
;             + END LOOP
;        END REGION

; CHECK: HIR Multiversioning for constant UB on function _Z3sublPf
; CHECK:      Give up due to non-integer blob value.
; CHECK-NOT: { modified } 


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x [7 x float]] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3sublPf(i64 noundef %N, float* nocapture noundef %M) local_unnamed_addr {
entry:
  %mul = shl nsw i64 %N, 1
  %cmp220 = icmp sgt i64 %mul, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc6
  %i.022 = phi i64 [ 0, %entry ], [ %inc7, %for.inc6 ]
  br i1 %cmp220, label %for.body3.preheader, label %for.inc6

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %k.021 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx4 = getelementptr inbounds [1000 x [7 x float]], [1000 x [7 x float]]* @A, i64 0, i64 %i.022, i64 %k.021
  %0 = load float, float* %arrayidx4, align 4
  %arrayidx5 = getelementptr inbounds float, float* %M, i64 %k.021
  %1 = load float, float* %arrayidx5, align 4
  %sub = fsub fast float %1, %0
  store float %sub, float* %arrayidx5, align 4
  %inc = add nuw nsw i64 %k.021, 1
  %exitcond.not = icmp eq i64 %inc, %mul
  br i1 %exitcond.not, label %for.inc6.loopexit, label %for.body3

for.inc6.loopexit:                                ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %inc7 = add nuw nsw i64 %i.022, 1
  %exitcond24.not = icmp eq i64 %inc7, 1000
  br i1 %exitcond24.not, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  %arrayidx9 = getelementptr inbounds float, float* %M, i64 %N
  %2 = load float, float* %arrayidx9, align 4
  %conv = fptosi float %2 to i32
  ret i32 %conv
}


