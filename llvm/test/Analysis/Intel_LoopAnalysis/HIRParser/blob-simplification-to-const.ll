; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; RUN: opt %s -passes="print<scalar-evolution>" -disable-output 2>&1 | FileCheck %s --check-prefix=SCEV


; Verify that framework is successfully able to handle simplification of blob
; to constant during reverse engineering of i2 loop upper bound.
; When parsing i2 loop upper, the AddRec {2,+,1}<nuw><%loop.outer>) is reverse
; engineered into %add.13 and smin(0, %add.13) is simplified to 0 because
; %add.13 is known to be greater than 0 due to nsw flag in the instruction.
; Since this flag is not propagated in AddRec form, the same cannot be deduced
; for the AddRec.


; SCEV: Loop %loop.inner: backedge-taken count is ((-1 * (0 smin {2,+,1}<nuw><%loop.outer>)) + {2,+,1}<nuw><%loop.outer>)

; CHECK: + DO i1 = 0, undef + -3, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, i1 + 2, 1   <DO_LOOP>
; CHECK: |   |   (%p)[0] = i1 + -1 * i2 + 2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %p) {
entry:
  br label %loop.outer

loop.outer:                                              ; preds = %outer.latch, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %outer.latch ], [ 2, %entry ]
  %iv.outer = phi i64 [ %add.13, %outer.latch ], [ 1, %entry ]
  %add.13 = add nuw nsw i64 %iv.outer, 1
  br label %loop.inner

loop.inner:                                   ; preds = %loop.inner, %loop.outer
  %iv.inner = phi i64 [ %add.13, %loop.outer ], [ %add.92, %loop.inner ]
  store i64 %iv.inner, ptr %p
  %rel.5 = icmp sgt i64 %iv.inner, 0
  %add.92 = add nsw i64 %iv.inner, -1
  br i1 %rel.5, label %loop.inner, label %outer.latch

outer.latch:                                             ; preds = %loop.inner
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %exitcond712 = icmp eq i64 %indvars.iv.next, undef
  br i1 %exitcond712, label %exit, label %loop.outer

exit:                                     ; preds = %outer.latch
  ret void
}

