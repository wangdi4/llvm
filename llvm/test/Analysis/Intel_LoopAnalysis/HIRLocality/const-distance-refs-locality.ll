; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-sorted-locality | FileCheck %s

; Src code-
;  for(i=2; i<N; i++)
;  for(j=2; j<N; j++)
;  for(k=2; k<N; k++)
;    A[i][j][k] += A[i-1][j+1][k+2] + A[i][j-2][k+1];


; Check locality values for the loopnest which has constant distance refs.

; CHECK: Locality Info for Loop level: 1     NumCacheLines: 55       SpatialCacheLines: 55    TempInvCacheLines: 0     AvgLvalStride: 400       AvgStride: 400
; CHECK: Locality Info for Loop level: 2     NumCacheLines: 10       SpatialCacheLines: 10    TempInvCacheLines: 0     AvgLvalStride: 40        AvgStride: 40
; CHECK: Locality Info for Loop level: 3     NumCacheLines: 3        SpatialCacheLines: 3     TempInvCacheLines: 0     AvgLvalStride: 4         AvgStride: 4


; ModuleID = 'loc.c'

@A = common local_unnamed_addr global [10 x [10 x [10 x i32]]] zeroinitializer, align 4

; Function Attrs: norecurse nounwind
define void @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp47 = icmp sgt i32 %N, 2
  br i1 %cmp47, label %for.cond4.preheader.lr.ph.preheader, label %for.end25

for.cond4.preheader.lr.ph.preheader:              ; preds = %entry
  br label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.cond4.preheader.lr.ph.preheader, %for.inc23
  %i.048 = phi i32 [ %inc24, %for.inc23 ], [ 2, %for.cond4.preheader.lr.ph.preheader ]
  %sub = add nsw i32 %i.048, -1
  br label %for.body6.lr.ph

for.cond1.loopexit:                               ; preds = %for.body6
  %exitcond50 = icmp eq i32 %add, %N
  br i1 %exitcond50, label %for.inc23, label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader.lr.ph, %for.cond1.loopexit
  %j.046 = phi i32 [ 2, %for.cond4.preheader.lr.ph ], [ %add, %for.cond1.loopexit ]
  %add = add nuw nsw i32 %j.046, 1
  %sub11 = add nsw i32 %j.046, -2
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.lr.ph
  %k.044 = phi i32 [ 2, %for.body6.lr.ph ], [ %add13, %for.body6 ]
  %add8 = add nuw nsw i32 %k.044, 2
  %arrayidx9 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @A, i32 0, i32 %sub, i32 %add, i32 %add8
  %0 = load i32, i32* %arrayidx9, align 4
  %add13 = add nuw nsw i32 %k.044, 1
  %arrayidx14 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @A, i32 0, i32 %i.048, i32 %sub11, i32 %add13
  %1 = load i32, i32* %arrayidx14, align 4
  %add15 = add nsw i32 %1, %0
  %arrayidx18 = getelementptr inbounds [10 x [10 x [10 x i32]]], [10 x [10 x [10 x i32]]]* @A, i32 0, i32 %i.048, i32 %j.046, i32 %k.044
  %2 = load i32, i32* %arrayidx18, align 4
  %add19 = add nsw i32 %add15, %2
  store i32 %add19, i32* %arrayidx18, align 4
  %exitcond = icmp eq i32 %add13, %N
  br i1 %exitcond, label %for.cond1.loopexit, label %for.body6

for.inc23:                                        ; preds = %for.cond1.loopexit
  %inc24 = add nuw nsw i32 %i.048, 1
  %exitcond51 = icmp eq i32 %inc24, %N
  br i1 %exitcond51, label %for.end25.loopexit, label %for.cond4.preheader.lr.ph

for.end25.loopexit:                               ; preds = %for.inc23
  br label %for.end25

for.end25:                                        ; preds = %for.end25.loopexit, %entry
  ret void
}

