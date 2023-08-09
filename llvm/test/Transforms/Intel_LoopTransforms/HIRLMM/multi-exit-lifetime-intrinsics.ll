; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm" -aa-pipeline="basic-aa" -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s

; Verify that we do not optimize store to (%B)[0][5] based on lifetime
; intrinsics in the multi-exit loop. There is a load to the same location in
; the loop exit block %for.body.for.end_crit_edge.

; CHECK: Dump Before

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   @llvm.lifetime.start.p0(4000,  &((%B)[0]));
; CHECK: |   %1 = (@A)[0][i1];
; CHECK: |   (%B)[0][5] = %1 + 1;
; CHECK: |   if (%1 > 0)
; CHECK: |   {
; CHECK: |      goto for.body.for.end_crit_edge;
; CHECK: |   }
; CHECK: |   @llvm.lifetime.end.p0(4000,  &((%B)[0]));
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, %N + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 1000>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   @llvm.lifetime.start.p0(4000,  &((%B)[0]));
; CHECK: |   %1 = (@A)[0][i1];
; CHECK: |   (%B)[0][5] = %1 + 1;
; CHECK: |   if (%1 > 0)
; CHECK: |   {
; CHECK: |      goto for.body.for.end_crit_edge;
; CHECK: |   }
; CHECK: |   @llvm.lifetime.end.p0(4000,  &((%B)[0]));
; CHECK: + END LOOP
; CHECK: END REGION


@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %cmp1 = icmp slt i32 0, %N
  %B = alloca [1000 x i32], align 16
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.cond
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %i.0, %for.cond ]
  call void @llvm.lifetime.start.p0(i64 4000, ptr nonnull %B)
  %0 = zext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %0
  %1 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %1, 1
  %arrayidxB = getelementptr inbounds [1000 x i32], ptr %B, i64 0, i64 5
  store i32 %add, ptr %arrayidxB, align 4
  %cmp3 = icmp sgt i32 %1, 0
  %inc = add nuw nsw i32 %i.02, 1
  br i1 %cmp3, label %for.body.for.end_crit_edge, label %for.cond

for.cond:                                         ; preds = %for.body
  %i.0 = phi i32 [ %inc, %for.body ]
  call void @llvm.lifetime.end.p0(i64 4000, ptr nonnull %B)
  %cmp = icmp slt i32 %i.0, %N
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.body.for.end_crit_edge:                       ; preds = %for.body
  %arrayidxB1 = getelementptr inbounds [1000 x i32], ptr %B, i64 0, i64 5
  %ld = load i32, ptr %arrayidxB1, align 4
  br label %for.end

for.cond.for.end_crit_edge:                       ; preds = %for.cond
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %for.body.for.end_crit_edge, %entry
  %phi = phi i32 [ 0, %entry ], [ 0, %for.cond.for.end_crit_edge ], [ %ld, %for.body.for.end_crit_edge ]
  ret i32 %phi
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }

