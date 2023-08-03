; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-cost-model-throttling=0  -hir-details < %s 2>&1 -disable-output | FileCheck %s

; Verify that the cloned and hoisted ub load (%LoopCount)[i1] becomes
; non-linear as base pointer is defined in outer loop

; HIR:
;
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %LoopCount = (%baseptrptr)[0];
; |   %0 = (%LoopCount)[i1];
; |   if (%0 > 0)
; |   {
; |      + UNKNOWN LOOP i2
; |      |   <i2 = 0>
; |      |   for.body4:
; |      |   %1 = (%A)[i2];
; |      |   (%A)[i2] = %1 + 100;
; |      |   %2 = (%LoopCount)[i1];
; |      |   if (i2 + 1 < %2)
; |      |   {
; |      |      <i2 = i2 + 1>
; |      |      goto for.body4;
; |      |   }
; |      + END LOOP
; |   }
; + END LOOP

; CHECK: + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %LoopCount = (%baseptrptr)[0];
; CHECK: |   %0 = (%LoopCount)[i1];
; CHECK: |   if (%0 > 0)
; CHECK: |   {
; CHECK: |      %2 = (%LoopCount)[i1];
; CHECK: |      <RVAL-REG> {al:4}(NON-LINEAR ptr %LoopCount)[LINEAR i64 i1]

; CHECK: |      %ub = smax(1, sext.i32.i64(%2)) + -1;
; CHECK: |      %mv.test = &((%A)[%ub]) >=u &((%LoopCount)[i1]);
; CHECK: |      %mv.test2 = &((%LoopCount)[i1]) >=u &((%A)[0]);
; CHECK: |      %mv.and = %mv.test  &  %mv.test2;
; CHECK: |      if (%mv.and == 0)  <MVTag: 35>
; CHECK: |      {
; CHECK: |         + DO i64 i2 = 0, %ub, 1   <DO_LOOP>  <MVTag: 35>
; CHECK: |         |   %1 = (%A)[i2];
; CHECK: |         |   (%A)[i2] = %1 + 100;
; CHECK: |         + END LOOP
; CHECK: |      }
; CHECK: |      else
; CHECK: |      {
; CHECK: |         + UNKNOWN LOOP i2  <MVTag: 35> <nounroll> <novectorize>
; CHECK: |         |   <i2 = 0>
; CHECK: |         |   for.body4.39:
; CHECK: |         |   %1 = (%A)[i2];
; CHECK: |         |   (%A)[i2] = %1 + 100;
; CHECK: |         |   %2 = (%LoopCount)[i1];
; CHECK: |         |   if (i2 + 1 < %2)
; CHECK: |         |   {
; CHECK: |         |      <i2 = i2 + 1>
; CHECK: |         |      goto for.body4.39;
; CHECK: |         |   }
; CHECK: |         + END LOOP
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fcnPiS_S_(ptr nocapture %A, ptr nocapture readonly %baseptrptr) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %indvars.iv19 = phi i64 [ 0, %entry ], [ %indvars.iv.next20, %for.cond.cleanup3 ]
  %LoopCount = load ptr, ptr %baseptrptr, align 4
  %arrayidx = getelementptr inbounds i32, ptr %LoopCount, i64 %indvars.iv19
  %0 = load i32, ptr %arrayidx, align 4
  %cmp216 = icmp sgt i32 %0, 0
  br i1 %cmp216, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next20, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx6 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6, align 4
  %add = add nsw i32 %1, 100
  store i32 %add, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %2 = load i32, ptr %arrayidx, align 4
  %3 = sext i32 %2 to i64
  %cmp2 = icmp slt i64 %indvars.iv.next, %3
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3.loopexit
}

