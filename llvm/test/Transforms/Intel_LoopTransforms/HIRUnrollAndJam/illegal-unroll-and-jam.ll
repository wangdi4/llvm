; RUN: opt < %s -hir-ssa-deconstruction -hir-unroll-and-jam -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s

; Verify that unroll & jam doesn't kick in for this case as it is illegal. The DV between A[i+1][j-1] and A[i][j] is (<, >) which yields an illegal DV after permutation.


; CHECK: Dump Before HIR Unroll & Jam

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@A)[0][i1][i2];
; CHECK: |   |   %1 = (@A)[0][i1 + 1][i2];
; CHECK: |   |   %add = %0  +  %1;
; CHECK: |   |   (@A)[0][i1 + 1][i2 + -1] = %add;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After HIR Unroll & Jam 

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   + DO i2 = 0, %m + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@A)[0][i1][i2];
; CHECK: |   |   %1 = (@A)[0][i1 + 1][i2];
; CHECK: |   |   %add = %0  +  %1;
; CHECK: |   |   (@A)[0][i1 + 1][i2 + -1] = %add;
; CHECK: |   + END LOOP
; CHECK: + END LOOP



source_filename = "illegal-unroll-and-jam.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %n, i64 %m) local_unnamed_addr #0 {
entry:
  %cmp25 = icmp sgt i64 %n, 0
  br i1 %cmp25, label %for.cond1.preheader.lr.ph, label %for.end12

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp223 = icmp sgt i64 %m, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc10, %for.cond1.preheader.lr.ph
  %i.026 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %add7, %for.inc10 ]
  %add7 = add nuw nsw i64 %i.026, 1
  br i1 %cmp223, label %for.body3.preheader, label %for.inc10

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %j.024 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx4 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %i.026, i64 %j.024
  %0 = load float, float* %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %add7, i64 %j.024
  %1 = load float, float* %arrayidx6, align 4
  %add = fadd float %0, %1
  %sub = add nsw i64 %j.024, -1
  %arrayidx9 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @A, i64 0, i64 %add7, i64 %sub
  store float %add, float* %arrayidx9, align 4
  %inc = add nuw nsw i64 %j.024, 1
  %exitcond = icmp eq i64 %inc, %m
  br i1 %exitcond, label %for.inc10.loopexit, label %for.body3

for.inc10.loopexit:                               ; preds = %for.body3
  br label %for.inc10

for.inc10:                                        ; preds = %for.inc10.loopexit, %for.cond1.preheader
  %exitcond28 = icmp eq i64 %add7, %n
  br i1 %exitcond28, label %for.end12.loopexit, label %for.cond1.preheader

for.end12.loopexit:                               ; preds = %for.inc10
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}

