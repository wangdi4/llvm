; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we skip loopnest multiversioning because TC is too large.

;   BEGIN REGION { }
;       + DO i1 = 0, 999, 1   <DO_LOOP>
;       |   if (%N > 4)
;       |   {
;       |      + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 16>  <LEGAL_MAX_TC = 2147483647>
;       |      |   %sub = (%M)[i2]  -  (@B)[0][i1][i2];
;       |      |   (%M)[i2] = %sub;
;       |      + END LOOP
;       |   }
;       + END LOOP
;   END REGION

; Dump after HIR multivection for constant UB from MAX_TC_EST.

;CHECK: BEGIN REGION { }
;CHECK-NOT:   if (%N == 16)
;CHECK:       + DO i1 = 0, 999, 1   <DO_LOOP>
;CHECK:       |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 16>  <LEGAL_MAX_TC = 2147483647>
;             |   |   %sub = (%M)[i2]  -  (@B)[0][i1][i2];
;             |   |   (%M)[i2] = %sub;
;             |   + END LOOP
;             + END LOOP
;       END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [1000 x [16 x float]] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @sub(i32 noundef %N, float* nocapture noundef %M) local_unnamed_addr {
entry:
  %cmp1 = icmp sgt i32 %N, 4
  %cmp324 = icmp slt i32 0, %N
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc9
  %i.026 = phi i32 [ 0, %entry ], [ %inc10, %for.inc9 ]
  br i1 %cmp1, label %for.cond2.preheader, label %for.inc9

for.cond2.preheader:                              ; preds = %for.body
  br i1 %cmp324, label %for.body4.lr.ph, label %for.inc9.loopexit

for.body4.lr.ph:                                  ; preds = %for.cond2.preheader
  %idxprom = zext i32 %i.026 to i64
  br label %for.body4

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %j.025 = phi i32 [ 0, %for.body4.lr.ph ], [ %inc, %for.body4 ]
  %idxprom5 = zext i32 %j.025 to i64
  %arrayidx6 = getelementptr inbounds [1000 x [16 x float]], [1000 x [16 x float]]* @B, i64 0, i64 %idxprom, i64 %idxprom5
  %0 = load float, float* %arrayidx6, align 4
  %arrayidx8 = getelementptr inbounds float, float* %M, i64 %idxprom5
  %1 = load float, float* %arrayidx8, align 4
  %sub = fsub fast float %1, %0
  store float %sub, float* %arrayidx8, align 4
  %inc = add nuw nsw i32 %j.025, 1
  %cmp3 = icmp slt i32 %inc, %N
  br i1 %cmp3, label %for.body4, label %for.cond2.for.inc9.loopexit_crit_edge

for.cond2.for.inc9.loopexit_crit_edge:            ; preds = %for.body4
  br label %for.inc9.loopexit

for.inc9.loopexit:                                ; preds = %for.cond2.for.inc9.loopexit_crit_edge, %for.cond2.preheader
  %i.028 = phi i32 [ %i.026, %for.cond2.for.inc9.loopexit_crit_edge ], [ %i.026, %for.cond2.preheader ]
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.body
  %i.027 = phi i32 [ %i.028, %for.inc9.loopexit ], [ %i.026, %for.body ]
  %inc10 = add nuw nsw i32 %i.027, 1
  %cmp = icmp ult i32 %inc10, 1000
  br i1 %cmp, label %for.body, label %for.end11

for.end11:                                        ; preds = %for.inc9
  %idxprom12 = sext i32 %N to i64
  %arrayidx13 = getelementptr inbounds float, float* %M, i64 %idxprom12
  %2 = load float, float* %arrayidx13, align 4
  %conv = fptosi float %2 to i32
  ret i32 %conv
}

