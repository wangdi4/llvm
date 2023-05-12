; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; The test is not working as expected since we started forming SCC for
; %phi and %mul.1.i.i.
; TODO: modify to test for the original intention.

; Verify that the assignment to %mul.lcssa.res after i2 loop is not
; parsed in terms of i2 loop's header phi %iv.inner as it causes live
; range violation. Previously, the parsing was like this-

; %iv.inner = 2;
; + DO i2 =
; |   %iv.inner = i2 + 3;
; + END LOOP
; %mul.lcssa.res = (%iv.inner * %phi.out);
;

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   %mul.lcssa.res = 1;
; CHECK: |   if (3 * i1 + -2 >=u 2)
; CHECK: |   {
; CHECK: |      %phi.root = 1;
; CHECK: |
; CHECK: |      + DO i2 = 0, smax(3, (1 + %iv.outer)) + -3, 1   <DO_LOOP>  <MAX_TC_EST = 18>  <LEGAL_MAX_TC = 18>
; CHECK: |      |   %phi.root = %phi.root  *  i2 + 2;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %mul.lcssa.res = %phi.root;
; CHECK: |   }
; CHECK: |   (i32*)(%argblock)[0].0 = %mul.lcssa.res;
; CHECK: |   %iv.outer = 3 * i1 + 1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %func_result, i32 %func_result2) local_unnamed_addr {
alloca_0:
  %argblock = alloca <{ i64 }>, align 8
  %BLKFIELD_i32_ = getelementptr inbounds <{ i64 }>, ptr %argblock, i64 0, i32 0
  br label %bb2

bb2:                                              ; preds = %bb8_endif, %alloca_0
  %iv.outer = phi i32 [ -2, %alloca_0 ], [ %add.1, %bb8_endif ]
  br label %bb_new25_else.i.i

bb11.i.i:                                         ; preds = %bb11.i.i.preheader, %bb11.i.i
  %iv.inner = phi i32 [ %add.2.i.i, %bb11.i.i ], [ 2, %bb11.i.i.preheader ]
  %phi = phi i32 [ %mul.1.i.i, %bb11.i.i ], [ 1, %bb11.i.i.preheader ]
  %mul.1.i.i = mul nsw i32 %phi, %iv.inner
  %add.2.i.i = add nuw nsw i32 %iv.inner, 1
  %rel.5.not.i.i = icmp sgt i32 %add.2.i.i, %iv.outer
  br i1 %rel.5.not.i.i, label %bb_new16_else.loopexit, label %bb11.i.i

bb_new25_else.i.i:                                ; preds = %bb2
  %rel.4.i.i = icmp ult i32 %iv.outer, 2
  br i1 %rel.4.i.i, label %bb_new16_else, label %bb11.i.i.preheader

bb11.i.i.preheader:                               ; preds = %bb_new25_else.i.i
  br label %bb11.i.i

bb_new16_else.loopexit:                           ; preds = %bb11.i.i
  %mul.1.i.i.lcssa = phi i32 [ %mul.1.i.i, %bb11.i.i ]
  br label %bb_new16_else

bb_new16_else:                                    ; preds = %bb_new16_else.loopexit, %bb_new25_else.i.i
  %mul.lcssa.res = phi i32 [ 1, %bb_new25_else.i.i ], [ %mul.1.i.i.lcssa, %bb_new16_else.loopexit ]
  store i32 %mul.lcssa.res, ptr %BLKFIELD_i32_, align 8
  br label %bb8_endif

bb8_endif:                                        ; preds = %bb_new16_else, %bb_new13_then
  %add.1 = add nsw i32 %iv.outer, 3
  %rel.2 = icmp slt i32 %iv.outer, 18
  br i1 %rel.2, label %bb2, label %bb5

bb5:                                              ; preds = %bb8_endif
  ret void
}

