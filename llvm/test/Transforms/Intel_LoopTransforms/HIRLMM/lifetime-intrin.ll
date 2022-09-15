; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm" -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s

; Verify that hir-lmm is able to replace store and load of (%A)[0][5] with temp
; by using the hint from lifetime.start/end intrinsics without hoisting 
; load/store to preheader/postexit.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   @llvm.lifetime.start.p0i8(40,  &((i8*)(%A)[0]));
; CHECK: |   (%A)[0][5] = (%src)[i1];
; CHECK: |   (%dst)[i1] = (%A)[0][5];
; CHECK: |   @llvm.lifetime.end.p0i8(40,  &((i8*)(%A)[0]));
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK-NOT: (%A)[0][5]
; CHECK:      + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NEXT: |   @llvm.lifetime.start.p0i8(40,  &((i8*)(%A)[0]));
; CHECK-NEXT: |   %limm = (%src)[i1];
; CHECK-NEXT: |   (%dst)[i1] = %limm;
; CHECK-NEXT: |   @llvm.lifetime.end.p0i8(40,  &((i8*)(%A)[0]));
; CHECK-NEXT: + END LOOP
; CHECK-NOT: (%A)[0][5]


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture noundef readonly %src, i32* nocapture noundef writeonly %dst, i32 noundef %n) {
entry:
  %A = alloca [10 x i32], align 16
  %cmp34 = icmp sgt i32 %n, 0
  br i1 %cmp34, label %for.body.lr.ph, label %for.end19

for.body.lr.ph:                                   ; preds = %entry
  %t0 = bitcast [10 x i32]* %A to i8*
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %indvars.iv41 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next42, %for.body ]
  call void @llvm.lifetime.start.p0i8(i64 40, i8* nonnull %t0) #2
  %arrayidx = getelementptr inbounds i32, i32* %src, i64 %indvars.iv41
  %t2 = load i32, i32* %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* %A, i64 0, i64 5
  store i32 %t2, i32* %arrayidx5, align 4
  %t3 = load i32, i32* %arrayidx5, align 4
  %arrayidx13 = getelementptr inbounds i32, i32* %dst, i64 %indvars.iv41
  store i32 %t3, i32* %arrayidx13, align 4
  call void @llvm.lifetime.end.p0i8(i64 40, i8* nonnull %t0) #2
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43.not = icmp eq i64 %indvars.iv.next42, %wide.trip.count
  br i1 %exitcond43.not, label %for.end19.loopexit, label %for.body

for.end19.loopexit:                               ; preds = %for.body
  br label %for.end19

for.end19:                                        ; preds = %for.end19.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

