; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-fusion -print-after=hir-loop-fusion -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Test checks that non-vectorizable first loop is not fused with vectorizable second one.

;   BEGIN REGION { }
;      + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;      |   %add = (@a)[0][i1]  +  (@c)[0][i1];
;      |   %2 = (@m)[0][i1];
;      |   (@a)[0][%2] = %add;
;      + END LOOP
;
;      + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;      |   %add14 = (@b)[0][i1]  +  (@c)[0][i1];
;      |   (@b)[0][i1] = %add14;
;      + END LOOP
;   END REGION

; CHECK-NOT: modified

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@m = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr {
entry:
  %cmp30 = icmp sgt i32 %n, 0
  br i1 %cmp30, label %for.body.preheader, label %for.end17

for.body.preheader:                               ; preds = %entry
  %wide.trip.count3638 = zext i32 %n to i64
  br label %for.body

for.body9.preheader:                              ; preds = %for.body
  br label %for.body9

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv33 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next34, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @c, i64 0, i64 %indvars.iv33
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [1000 x float], [1000 x float]* @a, i64 0, i64 %indvars.iv33
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd fast float %1, %0
  %arrayidx4 = getelementptr inbounds [100 x i32], [100 x i32]* @m, i64 0, i64 %indvars.iv33
  %2 = load i32, i32* %arrayidx4, align 4
  %idxprom5 = sext i32 %2 to i64
  %arrayidx6 = getelementptr inbounds [1000 x float], [1000 x float]* @a, i64 0, i64 %idxprom5
  store float %add, float* %arrayidx6, align 4
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond37.not = icmp eq i64 %indvars.iv.next34, %wide.trip.count3638
  br i1 %exitcond37.not, label %for.body9.preheader, label %for.body

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv = phi i64 [ 0, %for.body9.preheader ], [ %indvars.iv.next, %for.body9 ]
  %arrayidx11 = getelementptr inbounds [1000 x float], [1000 x float]* @c, i64 0, i64 %indvars.iv
  %3 = load float, float* %arrayidx11, align 4
  %arrayidx13 = getelementptr inbounds [1000 x float], [1000 x float]* @b, i64 0, i64 %indvars.iv
  %4 = load float, float* %arrayidx13, align 4
  %add14 = fadd fast float %4, %3
  store float %add14, float* %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count3638
  br i1 %exitcond.not, label %for.end17.loopexit, label %for.body9

for.end17.loopexit:                               ; preds = %for.body9
  br label %for.end17

for.end17:                                        ; preds = %for.end17.loopexit, %entry
  ret void
}
