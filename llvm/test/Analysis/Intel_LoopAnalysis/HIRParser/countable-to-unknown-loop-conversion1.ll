; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" 2>&1 | FileCheck %s

; Verify that we are able to parse this case successfully without assertion.
; The i2 loop was converted from countable to unknown as we moved from loop
; formation to parsing phase because of different results from ScalarEvolution.
; The no-wrap flags were different for the IV in the two queries.

; UPDATE: The i2 loop becomes countable after community pulldown. It is not
; known what changed.

; Old HIR-
; + DO i1 = 0, 39, 1   <DO_LOOP>
; |   + UNKNOWN LOOP i2
; |   |   <i2 = 0>
; |   |   for.body6:
; |   |   if (undef #UNDEF# undef)
; |   |   {
; |   |      goto for.end.loopexit;
; |   |   }
; |   |   if (i1 >=u 2 * i2 + trunc.i64.i32(%indvars.iv54) + 2)
; |   |   {
; |   |      <i2 = i2 + 1>
; |   |      goto for.body6;
; |   |   }
; |   + END LOOP
; |
; |   for.end.loopexit:
; + END LOOP

; New HIR-

; CHECK: + DO i1 = 0, 39, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, (-1 * trunc.i64.i32(%indvars.iv54) + umax((1 + trunc.i64.i32(%indvars.iv52)), (2 + trunc.i64.i32(%indvars.iv54))) + -1)/u2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   |   if (undef #UNDEF# undef)
; CHECK: |   |   {
; CHECK: |   |      goto for.end.loopexit;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   for.end.loopexit:
; CHECK: |   %indvars.iv52 = i1 + 1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @main() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end17, %entry
  %indvars.iv54 = phi i64 [ 20, %entry ], [ %indvars.iv.next55, %for.end17 ]
  %0 = trunc i64 %indvars.iv54 to i32
  br label %for.cond4.preheader

for.cond13.preheader:                             ; preds = %for.end.loopexit
  br i1 undef, label %for.body15.lr.ph, label %for.end17

for.body15.lr.ph:                                 ; preds = %for.cond13.preheader
  br label %for.body15

for.cond4.preheader:                              ; preds = %for.end.loopexit, %for.cond1.preheader
  %indvars.iv52 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next53, %for.end.loopexit ]
  br label %for.body6

for.body6:                                        ; preds = %for.inc, %for.cond4.preheader
  %storemerge2729 = phi i32 [ %add10, %for.inc ], [ %0, %for.cond4.preheader ]
  br i1 undef, label %for.end.loopexit, label %for.inc

for.inc:                                          ; preds = %for.body6
  %add10 = add nuw nsw i32 %storemerge2729, 2
  %1 = zext i32 %add10 to i64
  %cmp5.not = icmp ult i64 %indvars.iv52, %1
  br i1 %cmp5.not, label %for.end.loopexit, label %for.body6

for.end.loopexit:                                 ; preds = %for.inc, %for.body6
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next53, 40
  br i1 %exitcond.not, label %for.cond13.preheader, label %for.cond4.preheader

for.body15:                                       ; preds = %for.body15, %for.body15.lr.ph
  br i1 undef, label %for.body15, label %for.cond13.for.end17_crit_edge

for.cond13.for.end17_crit_edge:                   ; preds = %for.body15
  br label %for.end17

for.end17:                                        ; preds = %for.cond13.for.end17_crit_edge, %for.cond13.preheader
  %indvars.iv.next55 = add nsw i64 %indvars.iv54, -8
  %2 = trunc i64 %indvars.iv.next55 to i32
  br i1 false, label %for.cond1.preheader, label %for.end19

for.end19:                                        ; preds = %for.end17
  ret void
}



