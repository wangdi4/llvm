; RUN: opt -passes='print<scalar-evolution>' -scalar-evolution-print-scoped-mode < %s 2>&1 | FileCheck %s

; Verify that we are able to compute the trip count of %middle.loop in terms of
; outer.loop by tracing through the loop upper %outer.iv.phi which is an LCSSA phi.
; Previously, we were restricting traceback of all such phis in scoped mode but
; now we allow it for phis inside the region/scope.

; CHECK: Loop %inner.loop: backedge-taken count is {-1,+,1}<%outer.loop>
; CHECK: Loop %middle.loop: backedge-taken count is {-1,+,1}<%outer.loop>

define void @foo(ptr %ld.ptr) {
entry:
  br label %outer.loop

outer.loop:
  %outer.iv = phi i64 [ %outer.iv.inc, %outer.latch ], [ 0, %entry ]
  br label %middle.loop
 
middle.loop:                                             ; preds = %middle.latch, %outer.loop
  %middle.iv = phi i64 [ %middle.iv.inc, %middle.latch ], [ 0, %outer.loop ]
  br label %inner.loop

inner.loop:                                             ; preds = %inner.loop, %middle.loop
  %inner.iv = phi i64 [ 0, %middle.loop ], [ %inner.inv.inc, %inner.loop ]
  %inner.inv.inc = add nuw nsw i64 %inner.iv, 1
  %t11 = trunc i64 %inner.inv.inc to i32
  %outer.iv.trunc = trunc i64 %outer.iv to i32
  %cmp.inner = icmp eq i32 %t11, %outer.iv.trunc
  br i1 %cmp.inner, label %middle.latch, label %inner.loop

middle.latch:                                             ; preds = %inner.loop
  %outer.iv.phi = phi i32 [ %outer.iv.trunc, %inner.loop ]
  %middle.iv.inc = add nuw nsw i64 %middle.iv, 1
  %t21 = trunc i64 %middle.iv.inc to i32
  %cmp.middle = icmp eq i32 %t21, %outer.iv.phi
  br i1 %cmp.middle, label %outer.latch, label %middle.loop

outer.latch:                                             ; preds = %middle.latch
  %outer.iv.inc = add nuw nsw i64 %outer.iv, 1
  %ld = load i64, ptr %ld.ptr
  %cmp.outer = icmp eq i64 %outer.iv.inc, %ld
  br i1 %cmp.outer, label %exit, label %outer.loop

exit:
  ret void
}

