; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework | FileCheck %s

; Check that we are able to compute the max trip count estimate of the inner loop.

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 31>
; CHECK: |   |   (@A)[0][i1 + i2] = i1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; ModuleID = 'inner-loop-max-tc.ll'
source_filename = "max-tc3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [40 x i64] zeroinitializer, align 16

define void @foo(i64 %n) {
entry:
  %cmp213 = icmp sgt i64 %n, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc4, %entry
  %i.015 = phi i64 [ 0, %entry ], [ %inc5, %for.inc4 ]
  br i1 %cmp213, label %for.body3.preheader, label %for.inc4

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %j.014 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %add = add nuw nsw i64 %j.014, %i.015
  %arrayidx = getelementptr inbounds [40 x i64], [40 x i64]* @A, i64 0, i64 %add
  store i64 %i.015, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %j.014, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.inc4.loopexit, label %for.body3

for.inc4.loopexit:                                ; preds = %for.body3
  br label %for.inc4

for.inc4:                                         ; preds = %for.inc4.loopexit, %for.cond1.preheader
  %inc5 = add nuw nsw i64 %i.015, 1
  %exitcond17 = icmp eq i64 %inc5, 10
  br i1 %exitcond17, label %for.end6, label %for.cond1.preheader

for.end6:                                         ; preds = %for.inc4
  ret void
}
