; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-loop-distribute-memrec,print<hir-dd-analysis>" -hir-dd-analysis-verify=L2 -disable-output < %s 2>&1 | FileCheck %s

; Check that DD analysis is properly invalidated after prevec unrolling.
; The copy for %phi426.out is cleaned up after copy propagation and needs
; to be invalidated so later passes such as distribution get the correct
; DD info.

; HIR Before HIRPreVecUnroll
;           + DO i1 = 0, 0, 1   <DO_LOOP>
;           |   %phi426 = 0;
;           |
;           |   + DO i2 = 0, 16, 1   <DO_LOOP>
;           |   |   %phi430.out = 0;
;           |   |   %phi426.out = %phi426;
;           |   |   %select436 = (%phi426.out == 0) ? null : null;
;           |   |   %load439 = (%select436)[0];
;           |   |   %phi426 = %phi426.out;
;           |   + END LOOP
;           |
;           |   (null)[0] = 0;
;           |
;           |   + DO i2 = 0, 0, 1   <DO_LOOP>
;           |   |   %load520 = (null)[0];
;           |   + END LOOP
;           + END LOOP

; HIR After HIRPreVecUnroll
;           + DO i1 = 0, 0, 1   <DO_LOOP>
;           |   %phi426 = 0;
;           |
;           |   + DO i2 = 0, 16, 1   <DO_LOOP>
;           |   |   %phi430.out = 0;
;           |   |   %select436 = (%phi426 == 0) ? null : null;
;           |   |   %load439 = (%select436)[0];
;           |   |   %phi426 = %phi426;
;           |   + END LOOP
;           |
;           |   (null)[0] = 0;
;           |   %load520 = (null)[0];
;           + END LOOP

; 14:10 %phi426 --> %phi426 FLOW (<= *) (? ?)
; 14:14 %phi426 --> %phi426 FLOW (<= *) (? ?)
; 10:14 %phi426 --> %phi426 ANTI (= =) (0 0)
; 10:11 %select436 --> %select436 FLOW (= =) (0 0)
; 14:14 %phi426 --> %phi426 ANTI (= =) (0 0)

; CHECK: DD graph for function wombat.bb402:
; CHECK-NOT: %phi426.out

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @wombat.bb402() {
newFuncRoot:
  br label %bb409

bb409:                                            ; preds = %bb527, %newFuncRoot
  br label %bb425

bb425:                                            ; preds = %bb425, %bb409
  %phi426 = phi i32 [ 0, %bb409 ], [ %phi426, %bb425 ]
  %phi4271 = phi i64 [ 0, %bb409 ], [ %add487, %bb425 ]
  %phi430 = phi i32 [ 0, %bb409 ], [ 0, %bb425 ]
  %icmp431 = icmp eq i32 %phi426, 0
  %select436 = select i1 %icmp431, ptr null, ptr null
  %load439 = load i8, ptr %select436, align 2
  %add487 = add i64 %phi4271, 1
  %icmp488 = icmp eq i64 %phi4271, 16
  br i1 %icmp488, label %bb489, label %bb425

bb489:                                            ; preds = %bb425
  store i32 0, ptr null, align 4
  br label %bb515

bb515:                                            ; preds = %bb515, %bb489
  %phi516 = phi i1 [ false, %bb489 ], [ false, %bb515 ]
  %load520 = load i32, ptr null, align 4
  br i1 %phi516, label %bb515, label %bb527

bb527:                                            ; preds = %bb515
  %icmp569 = icmp eq i64 1, 1
  br i1 %icmp569, label %bb570, label %bb409

bb570:                                            ; preds = %bb527
  %phi571 = phi i32 [ %phi430, %bb527 ]
  ret void
}
