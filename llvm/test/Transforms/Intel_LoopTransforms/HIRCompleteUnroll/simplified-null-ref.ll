; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll < %s 2>&1 | FileCheck %s

; Verify that the test compiles successfully. After unrolling we simplified the
; loads (@.ref.tmp)[0][0] and (@.ref.tmp)[0][1] to null but the CE that was
; created to represent the null was created by forming a blob for the null value
; instead of creating a '0' value ptr type CE. We now have verification logic to
; make sure we don't make this mistake elsewhere.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, -1 * %0, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll>
; CHECK: |   |   %1 = (@.ref.tmp)[0][i2];
; CHECK: |   |   (%ArrayNum216)[0] = &((%1)[0]);
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, -1 * %0, 1   <DO_LOOP>
; CHECK: |   %1 = null;
; CHECK: |   (%ArrayNum216)[0] = &((%1)[0]);
; CHECK: |   %1 = null;
; CHECK: |   (%ArrayNum216)[0] = &((%1)[0]);
; CHECK: + END LOOP


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.37.0"

@.ref.tmp = constant [2 x ptr] zeroinitializer

define void @foo(ptr %ArrayNum216, i32 %0) {
entry:
  br label %loop.outer

loop.outer:                              ; preds = %outer.latch, %entry
  %count = phi i32 [ %dec242, %outer.latch ], [ %0, %entry ]
  br label %loop.inner

loop.inner:                               ; preds = %loop.inner, %loop.outer
  %ptr.phi = phi ptr [ %ptr.inc, %loop.inner ], [ @.ref.tmp, %loop.outer ]
  %1 = load ptr, ptr %ptr.phi, align 8
  store ptr %1, ptr %ArrayNum216, align 8
  %ptr.inc = getelementptr ptr, ptr %ptr.phi, i64 1
  %cmp.not = icmp eq ptr %ptr.inc, getelementptr inbounds ([2 x ptr], ptr @.ref.tmp, i64 1, i64 0)
  br i1 %cmp.not, label %outer.latch, label %loop.inner, !llvm.loop !0

outer.latch: ; preds = %loop.inner
  %dec242 = add i32 %count, 1
  %tobool.not243 = icmp eq i32 %count, 0
  br i1 %tobool.not243, label %exit, label %loop.outer

exit: ; preds = %outer.latch
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}

