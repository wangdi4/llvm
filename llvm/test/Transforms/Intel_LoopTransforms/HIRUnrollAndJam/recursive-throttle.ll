; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we recursively throttle unroll & jam of i2 and i1 loops due to presence of if condition.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   if (i2 >u 20)
; CHECK: |   |   {
; CHECK: |   |      %0 = (@A)[0][i1][i2];
; CHECK: |   |
; CHECK: |   |      + DO i3 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |      |   %mul = (@B)[0][i2][i3]  *  (@C)[0][i3][i2];
; CHECK: |   |      |   %0 = %0  +  %mul;
; CHECK: |   |      + END LOOP
; CHECK: |   |
; CHECK: |   |      (@A)[0][i1][i2] = %0;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc16, %entry
  %i.035 = phi i64 [ 0, %entry ], [ %inc17, %for.inc16 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc13, %for.cond1.preheader
  %j.033 = phi i64 [ 0, %for.cond1.preheader ], [ %inc14, %for.inc13 ]
  %cmp4 = icmp ugt i64 %j.033, 20
  br i1 %cmp4, label %for.cond5.preheader, label %for.inc13

for.cond5.preheader:                              ; preds = %for.body3
  %arrayidx12 = getelementptr inbounds [100 x [100 x float]], ptr @A, i64 0, i64 %i.035, i64 %j.033
  %arrayidx12.promoted = load float, ptr %arrayidx12, align 4
  br label %for.body7

for.body7:                                        ; preds = %for.body7, %for.cond5.preheader
  %0 = phi float [ %arrayidx12.promoted, %for.cond5.preheader ], [ %add, %for.body7 ]
  %k.032 = phi i64 [ 0, %for.cond5.preheader ], [ %inc, %for.body7 ]
  %arrayidx8 = getelementptr inbounds [100 x [100 x float]], ptr @B, i64 0, i64 %j.033, i64 %k.032
  %1 = load float, ptr %arrayidx8, align 4
  %arrayidx10 = getelementptr inbounds [100 x [100 x float]], ptr @C, i64 0, i64 %k.032, i64 %j.033
  %2 = load float, ptr %arrayidx10, align 4
  %mul = fmul float %1, %2
  %add = fadd float %0, %mul
  %inc = add nuw nsw i64 %k.032, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.inc13.loopexit, label %for.body7

for.inc13.loopexit:                               ; preds = %for.body7
  %add.lcssa = phi float [ %add, %for.body7 ]
  store float %add.lcssa, ptr %arrayidx12, align 4
  br label %for.inc13

for.inc13:                                        ; preds = %for.inc13.loopexit, %for.body3
  %inc14 = add nuw nsw i64 %j.033, 1
  %exitcond36 = icmp eq i64 %inc14, 100
  br i1 %exitcond36, label %for.inc16, label %for.body3

for.inc16:                                        ; preds = %for.inc13
  %inc17 = add nuw nsw i64 %i.035, 1
  %exitcond37 = icmp eq i64 %inc17, 100
  br i1 %exitcond37, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.inc16
  ret void
}
