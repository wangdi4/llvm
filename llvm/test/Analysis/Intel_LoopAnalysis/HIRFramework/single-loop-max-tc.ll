; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework | FileCheck %s

; Check that we are able to compute the max trip count estimate of the loop.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 40>
; CHECK: |   {al:8}(@A)[0][i1] = i1;
; CHECK: + END LOOP

; ModuleID = 'max-tc.ll'
source_filename = "max-tc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [40 x i64] zeroinitializer, align 16

define void @foo(i64 %n) {
entry:
  %cmp5 = icmp sgt i64 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [40 x i64], [40 x i64]* @A, i64 0, i64 %i.06
  store i64 %i.06, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %i.06, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
