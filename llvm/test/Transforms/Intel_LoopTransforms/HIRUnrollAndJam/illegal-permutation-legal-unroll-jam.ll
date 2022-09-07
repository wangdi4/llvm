; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -hir-unroll-and-jam-max-factor=2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s --check-prefix=CHECK-DD

; Verify that i1 loop can be unroll & jammed in the presence of this edge which is
; loop independant at the i1 level but is not legal to permute i1-i3 level-

; CHECK-DD: (@A)[0][i1][i2 + 1][i3] --> (@A)[0][i1][i2][i3 + 1] FLOW (= < >)

; Input HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 98, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 98, 1   <DO_LOOP>
; |   |   |   %1 = (@A)[0][i1][i2][i3 + 1];
; |   |   |   %2 = (@C)[0][i2 + 1][i3];
; |   |   |   (@A)[0][i1][i2 + 1][i3] = %1 + %2;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 98, 1   <DO_LOOP>
; CHECK: |   |   |   %1 = (@A)[0][2 * i1][i2][i3 + 1];
; CHECK: |   |   |   %2 = (@C)[0][i2 + 1][i3];
; CHECK: |   |   |   (@A)[0][2 * i1][i2 + 1][i3] = %1 + %2;
; CHECK: |   |   |   %1 = (@A)[0][2 * i1 + 1][i2][i3 + 1];
; CHECK: |   |   |   %2 = (@C)[0][i2 + 1][i3];
; CHECK: |   |   |   (@A)[0][2 * i1 + 1][i2 + 1][i3] = %1 + %2;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x [100 x i32]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [100 x [100 x i32]]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc25
  %indvars.iv49 = phi i64 [ 0, %entry ], [ %indvars.iv.next50, %for.inc25 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc22
  %indvars.iv45 = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next46, %for.inc22 ]
  %0 = add nsw i64 %indvars.iv45, -1
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* @A, i64 0, i64 %indvars.iv49, i64 %0, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx10, align 4
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @C, i64 0, i64 %indvars.iv45, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx14, align 4
  %add15 = add nsw i32 %2, %1
  %arrayidx21 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* @A, i64 0, i64 %indvars.iv49, i64 %indvars.iv45, i64 %indvars.iv
  store i32 %add15, i32* %arrayidx21, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, 99
  br i1 %exitcond.not, label %for.inc22, label %for.body6

for.inc22:                                        ; preds = %for.body6
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond48.not = icmp eq i64 %indvars.iv.next46, 100
  br i1 %exitcond48.not, label %for.inc25, label %for.cond4.preheader

for.inc25:                                        ; preds = %for.inc22
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond51.not = icmp eq i64 %indvars.iv.next50, 100
  br i1 %exitcond51.not, label %for.end27, label %for.cond1.preheader

for.end27:                                        ; preds = %for.inc25
  ret void
}

