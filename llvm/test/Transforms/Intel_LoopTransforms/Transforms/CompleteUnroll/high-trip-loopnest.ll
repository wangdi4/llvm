
; Verify that either only the inner loop or the complete loopnest (trip count of 56) is unrolled based on trip threshold.

; HIR -
; + DO i1 = 0, 7, 1   <DO_LOOP>
; |   + DO i2 = 0, 6, 1   <DO_LOOP>
; |   |   %1 = {al:4}(@A)[0][i1][i2];
; |   |   {al:4}(@A)[0][i1][i2] = i1 + i2 + %1;
; |   + END LOOP
; + END LOOP

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -hir-complete-unroll-trip-threshold=55 -print-before=hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck --check-prefix=INNER %s

; INNER: Dump Before HIR Complete Unroll
; INNER: DO i1
; INNER: DO i2


; INNER: Dump After HIR Complete Unroll
; INNER: DO i1
; INNER-NOT: DO i2

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -hir-complete-unroll-trip-threshold=56 -print-before=hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck --check-prefix=OUTER %s

; OUTER: Dump Before HIR Complete Unroll
; OUTER: DO i1
; OUTER: DO i2


; OUTER: Dump After HIR Complete Unroll
; OUTER-NOT: DO i1
; OUTER-NOT: DO i2


source_filename = "high-trip-loopnest.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external global [50 x [50 x i32]], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv20
  %arrayidx5 = getelementptr inbounds [50 x [50 x i32]], [50 x [50 x i32]]* @A, i64 0, i64 %indvars.iv20, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %2 = trunc i64 %0 to i32
  %add6 = add nsw i32 %2, %1
  store i32 %add6, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 7
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 8
  br i1 %exitcond22, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

