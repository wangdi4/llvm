; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework | FileCheck %s

; Check that we are able to compute the max trip count estimate of the outer loop.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>
; CHECK: |   + DO i2 = 0, 19, 1   <DO_LOOP>
; CHECK: |   |   (@A)[0][-1 * i1 + i2] = i1 + i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; ModuleID = 'outer-loop-neg-iv-max-tc.ll'
source_filename = "outer-max-tc1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [40 x i64] zeroinitializer, align 16

define void @foo(i64 %n) {
entry:
  %cmp16 = icmp sgt i64 %n, 0
  br i1 %cmp16, label %for.cond1.preheader.preheader, label %for.end7

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc5
  %i.017 = phi i64 [ %inc6, %for.inc5 ], [ 0, %for.cond1.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.015 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %add = add nuw nsw i64 %j.015, %i.017
  %add4 = sub nsw i64 %j.015, %i.017
  %arrayidx = getelementptr inbounds [40 x i64], [40 x i64]* @A, i64 0, i64 %add4
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %j.015, 1
  %exitcond = icmp eq i64 %inc, 20
  br i1 %exitcond, label %for.inc5, label %for.body3

for.inc5:                                         ; preds = %for.body3
  %inc6 = add nuw nsw i64 %i.017, 1
  %exitcond18 = icmp eq i64 %inc6, %n
  br i1 %exitcond18, label %for.end7.loopexit, label %for.cond1.preheader

for.end7.loopexit:                                ; preds = %for.inc5
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}
