; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -hir-unroll-and-jam-max-factor=2 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s --check-prefix=CHECK-DD

; Check that we can unroll & jam i1 loop even with this edge between sibling
; loops:

; CHECK-DD: (@A)[0][i1][i2] --> (@A)[0][i1][i2 + 1] FLOW (=)

; HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP> <ivdep>
; |   + DO i2 = 0, 99, 1   <DO_LOOP>
; |   |   %ld = (@C)[0][i2];
; |   |   (@A)[0][i1][i2] = %ld;
; |   + END LOOP
; |
; |
; |   + DO i2 = 0, 98, 1   <DO_LOOP>
; |   |   %ld1 = (@A)[0][i1][i2 + 1];
; |   |   (@B)[0][i1][i2] = %ld1;
; |   + END LOOP
; + END LOOP


; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam> <ivdep>
; CHECK: |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK: |   |   %ld = (@C)[0][i2];
; CHECK: |   |   (@A)[0][2 * i1][i2] = %ld;
; CHECK: |   |   %ld = (@C)[0][i2];
; CHECK: |   |   (@A)[0][2 * i1 + 1][i2] = %ld;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK: |   |   %ld1 = (@A)[0][2 * i1][i2 + 1];
; CHECK: |   |   (@B)[0][2 * i1][i2] = %ld1;
; CHECK: |   |   %ld1 = (@A)[0][2 * i1 + 1][i2 + 1];
; CHECK: |   |   (@B)[0][2 * i1 + 1][i2] = %ld1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc17
  %indvars.iv38 = phi i64 [ 0, %entry ], [ %indvars.iv.next39, %for.inc17 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %cgep = getelementptr inbounds [100 x i32], [100 x i32]* @C, i64 0, i64 %indvars.iv
  %ld = load i32, i32* %cgep, align 4
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv38, i64 %indvars.iv
  store i32 %ld, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.body6.preheader, label %for.body3

for.body6.preheader:                              ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv35 = phi i64 [ %indvars.iv.next36, %for.body6 ], [ 0, %for.body6.preheader ]
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv38, i64 %indvars.iv.next36
  %ld1 = load i32, i32* %arrayidx8, align 4
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv38, i64 %indvars.iv35
  store i32 %ld1, i32* %arrayidx12, align 4
  %exitcond37.not = icmp eq i64 %indvars.iv.next36, 99
  br i1 %exitcond37.not, label %for.inc17, label %for.body6

for.inc17:                                        ; preds = %for.body6
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond40.not = icmp eq i64 %indvars.iv.next39, 100
  br i1 %exitcond40.not, label %for.end19, label %for.cond1.preheader, !llvm.loop !0

for.end19:                                        ; preds = %for.inc17
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.ivdep_loop"}

