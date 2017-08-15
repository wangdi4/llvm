; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-locality-analysis -hir-sorted-locality | FileCheck %s

; Verify that sorted locality is able to handle denominators in the subscripts successfully.

; HIR-
; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   + DO i2 = 0, 49, 1   <DO_LOOP>
; |   |   %0 = (%B)[i1];
; |   |   %1 = (%A)[(i2)/u2];
; |   |   (%A)[(i2)/u2] = %0 + %1;
; |   + END LOOP
; + END LOOP


; CHECK: Locality Info for Loop level: 2     NumCacheLines: 3        SpatialCacheLines: 2     TempInvCacheLines: 1     AvgLvalStride: 2         AvgStride: 1
; CHECK: Locality Info for Loop level: 1     NumCacheLines: 2        SpatialCacheLines: 1     TempInvCacheLines: 1     AvgLvalStride: 0         AvgStride: 2


; ModuleID = 'udiv.c'
source_filename = "udiv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %A, i32* nocapture readonly %B) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc6 ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.015 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %0 = load i32, i32* %arrayidx, align 4
  %div = lshr i32 %j.015, 1
  %idxprom4 = zext i32 %div to i64
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %idxprom4
  %1 = load i32, i32* %arrayidx5, align 4
  %add = add i32 %1, %0
  store i32 %add, i32* %arrayidx5, align 4
  %inc = add nuw nsw i32 %j.015, 1
  %exitcond = icmp eq i32 %inc, 50
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond17 = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond17, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  ret void
}

