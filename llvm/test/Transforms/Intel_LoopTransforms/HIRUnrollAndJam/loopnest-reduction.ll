; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we can support loopnest reductions by skipping temp renaming.

; CHECK: Function: foo

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%A)[i1][i2];
; CHECK: |   |   %1 = (%B)[i1][i2];
; CHECK: |   |   %r.026 = (%1 * %0)  +  %r.026;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function: foo

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 24, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%A)[4 * i1][i2];
; CHECK: |   |   %1 = (%B)[4 * i1][i2];
; CHECK: |   |   %r.026 = (%1 * %0)  +  %r.026;
; CHECK: |   |   %0 = (%A)[4 * i1 + 1][i2];
; CHECK: |   |   %1 = (%B)[4 * i1 + 1][i2];
; CHECK: |   |   %r.026 = (%1 * %0)  +  %r.026;
; CHECK: |   |   %0 = (%A)[4 * i1 + 2][i2];
; CHECK: |   |   %1 = (%B)[4 * i1 + 2][i2];
; CHECK: |   |   %r.026 = (%1 * %0)  +  %r.026;
; CHECK: |   |   %0 = (%A)[4 * i1 + 3][i2];
; CHECK: |   |   %1 = (%B)[4 * i1 + 3][i2];
; CHECK: |   |   %r.026 = (%1 * %0)  +  %r.026;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(ptr nocapture readonly %A, ptr nocapture readonly %B) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.cond.cleanup3 ]
  %r.026 = phi i32 [ 0, %entry ], [ %add.lcssa, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.cond.cleanup3 ]
  ret i32 %add.lcssa.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi i32 [ %add, %for.body4 ]
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, 100
  br i1 %exitcond30, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !0

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %r.124 = phi i32 [ %r.026, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx6 = getelementptr inbounds [1000 x i32], ptr %A, i64 %indvars.iv28, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx6, align 4
  %arrayidx10 = getelementptr inbounds [1000 x i32], ptr %B, i64 %indvars.iv28, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx10, align 4
  %mul = mul nsw i32 %1, %0
  %add = add nsw i32 %mul, %r.124
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll_and_jam.count", i32 4}
