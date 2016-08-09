; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework | FileCheck %s

; Check that we are able to compute the max trip count estimate of the inner loop of the triangular loopnest.

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, i1 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>
; CHECK: |   |   (@A)[0][i1 + i2] = i1 + i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; ModuleID = 'tri-max-tc.ll'
source_filename = "tri-max-tc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [40 x i64] zeroinitializer, align 16

define void @foo(i64 %n) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc5, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc5 ]
  %cmp216 = icmp sgt i64 %indvars.iv, 0
  br i1 %cmp216, label %for.body3.preheader, label %for.inc5

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %j.017 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %add = add nuw nsw i64 %j.017, %indvars.iv
  %arrayidx = getelementptr inbounds [40 x i64], [40 x i64]* @A, i64 0, i64 %add
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %j.017, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv
  br i1 %exitcond, label %for.inc5.loopexit, label %for.body3

for.inc5.loopexit:                                ; preds = %for.body3
  br label %for.inc5

for.inc5:                                         ; preds = %for.inc5.loopexit, %for.cond1.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond20 = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond20, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  ret void
}
