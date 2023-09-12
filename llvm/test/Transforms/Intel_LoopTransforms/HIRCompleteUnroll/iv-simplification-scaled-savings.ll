; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that to account for the savings resulting from simpliciations of IVs
; i1, i2 and i3, we only consider the partial loopnest starting with that IV
; level.

; Savings for simplifying i1 = 5
; Savings for simplifying i2 = 5 * 5 = 25
; Savings for simplifying i3 = 5 * 5 * 5 = 125
; Savings for i1 and i2 go into 'ScaledSavings' component to account for partial
; loponest trip count and savings for i3 go into regular 'Savings' component as
; the entire loopnest trip count applies to it.

; CHECK: Savings: 125
; CHECK: ScaledSavings: 30

; TODO: we need to do the same thing for address simplifications as it is very
; high right now.

; CHECK: GEPSavings: 500

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   |   %0 = (@A)[0][i1][i2][i3];
; CHECK: |   |   |   %t.033 = %0  +  %t.033; <Safe Reduction>
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %t.033.out = %t.033;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [5 x [5 x [5 x i32]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(read, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc14
  %indvars.iv37 = phi i64 [ 0, %entry ], [ %indvars.iv.next38, %for.inc14 ]
  %t.033 = phi i32 [ 0, %entry ], [ %add.lcssa.lcssa, %for.inc14 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc11
  %indvars.iv34 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next35, %for.inc11 ]
  %t.131 = phi i32 [ %t.033, %for.cond1.preheader ], [ %add.lcssa, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %t.229 = phi i32 [ %t.131, %for.cond4.preheader ], [ %add, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [5 x [5 x [5 x i32]]], [5 x [5 x [5 x i32]]]* @A, i64 0, i64 %indvars.iv37, i64 %indvars.iv34, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx10, align 4
  %add = add nsw i32 %0, %t.229
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond.not, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %add.lcssa = phi i32 [ %add, %for.body6 ]
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond36.not = icmp eq i64 %indvars.iv.next35, 5
  br i1 %exitcond36.not, label %for.inc14, label %for.cond4.preheader

for.inc14:                                        ; preds = %for.inc11
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.inc11 ]
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond39.not = icmp eq i64 %indvars.iv.next38, 5
  br i1 %exitcond39.not, label %for.end16, label %for.cond1.preheader

for.end16:                                        ; preds = %for.inc14
  %add.lcssa.lcssa.lcssa = phi i32 [ %add.lcssa.lcssa, %for.inc14 ]
  ret i32 %add.lcssa.lcssa.lcssa
}

