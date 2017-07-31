; Verify that the complete triangular loopnest (avg trip count of 50) is unrolled based on trip threshold.

; HIR -
; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   + DO i2 = 0, i1 + -1, 1   <DO_LOOP>
; |   |   %0 = (@A)[0][i1][i2];
; |   |   (@A)[0][i1][i2] = i1 + i2 + %0;
; |   + END LOOP
; + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-complete-unroll-loop-trip-threshold=10 -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck --check-prefix=NO-UNROLL %s

; NO-UNROLL: Dump Before HIR PostVec Complete Unroll
; NO-UNROLL: DO i1
; NO-UNROLL: DO i2


; NO-UNROLL: Dump After HIR PostVec Complete Unroll
; NO-UNROLL: DO i1
; NO-UNROLL: DO i2

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-complete-unroll-loop-trip-threshold=50 -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck --check-prefix=UNROLL %s

; UNROLL: Dump Before HIR PostVec Complete Unroll
; UNROLL: DO i1
; UNROLL: DO i2


; UNROLL: Dump After HIR PostVec Complete Unroll
; UNROLL-NOT: DO i1
; UNROLL-NOT: DO i2


source_filename = "high-trip-loopnest1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external global [50 x [50 x i64]], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc6 ]
  %cmp217 = icmp sgt i64 %indvars.iv, 0
  br i1 %cmp217, label %for.body3.preheader, label %for.inc6

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %j.018 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %add = add nuw nsw i64 %j.018, %indvars.iv
  %arrayidx4 = getelementptr inbounds [50 x [50 x i64]], [50 x [50 x i64]]* @A, i64 0, i64 %indvars.iv, i64 %j.018
  %0 = load i64, i64* %arrayidx4, align 8
  %add5 = add nsw i64 %add, %0
  store i64 %add5, i64* %arrayidx4, align 8
  %inc = add nuw nsw i64 %j.018, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv
  br i1 %exitcond, label %for.inc6.loopexit, label %for.body3

for.inc6.loopexit:                                ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond21 = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond21, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  ret void
}

