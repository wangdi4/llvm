; RUN: opt < %s -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that the test case compiles successfully. The redundant condition
; %cmp1135 along with the loop for.cond13.preheader that it is guarding was
; optimized away by HIRCleanup phase. This missing loop was resulting in an
; assertion in HIRScalarSymbaseAssignment phase.

; CHECK: + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   %ld = (%ptr)[0];
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @main(ptr %ptr) local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc21, %entry
  %inc2240 = phi i32 [ 0, %for.inc21 ], [ undef, %entry ]
  %ld = load i32, ptr %ptr
  %cmp = icmp sgt i32 %inc2240, 16
  br i1 %cmp, label %for.cond1thread-pre-split, label %for.inc21

for.cond1thread-pre-split:                        ; preds = %for.body
  br label %for.cond4thread-pre-split.preheader

for.cond4thread-pre-split.preheader:              ; preds = %for.cond1thread-pre-split
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond4thread-pre-split.preheader
  %cmp1135 = icmp slt i32 %inc2240, 8
  br i1 %cmp1135, label %for.cond13.preheader.lr.ph, label %for.inc21.loopexit

for.cond13.preheader.lr.ph:                       ; preds = %for.cond10.preheader
  br label %for.cond13.preheader

for.cond13.preheader:                             ; preds = %for.inc18, %for.cond13.preheader.lr.ph
  %mul32.lcssa38 = phi i32 [ undef, %for.cond13.preheader.lr.ph ], [ %mul.lcssa, %for.inc18 ]
  %storemerge36 = phi i32 [ 8, %for.cond13.preheader.lr.ph ], [ %dec19, %for.inc18 ]
  %mul = mul nsw i32 %mul32.lcssa38, %inc2240
  br label %for.inc18

for.inc18:                                        ; preds = %for.cond13.preheader
  %mul.lcssa = phi i32 [ %mul, %for.cond13.preheader ]
  %dec19 = add nsw i32 %storemerge36, -1
  %cmp11 = icmp sgt i32 %dec19, %inc2240
  br i1 %cmp11, label %for.cond13.preheader, label %for.cond10.for.inc21.loopexit_crit_edge

for.cond10.for.inc21.loopexit_crit_edge:          ; preds = %for.inc18
  br label %for.inc21.loopexit

for.inc21.loopexit:                               ; preds = %for.cond10.for.inc21.loopexit_crit_edge, %for.cond10.preheader
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21.loopexit, %for.body
  %tobool.not = icmp eq i32 0, 0
  br i1 %tobool.not, label %for.cond.for.end23_crit_edge, label %for.body

for.cond.for.end23_crit_edge:                     ; preds = %for.inc21
  ret void
}

