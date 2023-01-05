; RUN: opt < %s -passes='module(intel-loop-attrs)' -force-intel-must-progress -S 2>&1 | FileCheck %s

; This test case checks that the loop in @foo is marked "mustprogress" since
; it has bounds. Function @foo should not be marked "mustprogress" since
; there is a call.  This is the same test case as intel_loopattrs02.ll but it
; checks the IR.

; CHECK: define i32 @foo(i32 %0, i32 %1)
; CHECK: br i1 %6, label %after.loop, label %loop.start, !llvm.loop !0

; CHECK-NOT: attributes #0 = { mustprogress }
; CHECK: !0 = distinct !{!0, !1}
; CHECK: !1 = !{!"llvm.loop.mustprogress"}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @baz(i32 %0)

define i32 @foo(i32 %0, i32 %1) {
2:
  %3 = icmp sgt i32 %0, %1
  br i1 %3, label %after.loop, label %loop.start

loop.start:
  %4 = phi i32 [ 0, %2], [ %5, %loop.start]
  %5 = add nuw nsw i32 %4, 1
  %6 = icmp sgt i32 %5, %1
  br i1 %6, label %after.loop, label %loop.start

after.loop:
  %7 = phi i32 [ 0, %2], [ %5, %loop.start]
  %8 = call i32 @baz(i32 %7)
  ret i32 %8
}
