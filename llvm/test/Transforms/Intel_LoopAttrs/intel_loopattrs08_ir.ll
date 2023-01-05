; RUN: opt < %s -passes='module(intel-loop-attrs)' -S 2>&1 | FileCheck %s

; This test case checks that the loop in @foo wasn't marked "mustprogress"
; since there is a call inside one of the basic blocks. Also, function @foo
; shouldn't be marked "mustprogress". This is the same test case as
; intel_loopattrs08.ll but it checks the IR.

; CHECK: define i32 @foo(i32 %0, i32 %1) {
; CHECK-NOT: attributes #0 = { mustprogress }
; CHECK-NOT: !0 = distinct !{!0, !1}
; CHECK-NOT: !1 = !{!"llvm.loop.mustprogress"}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @bar()

define i32 @foo(i32 %0, i32 %1) {
2:
  %3 = icmp sgt i32 %0, %1
  br i1 %3, label %after.loop, label %loop.start

loop.start:
  %4 = phi i32 [ 0, %2], [ %5, %loop.continue]
  %5 = add nuw nsw i32 %4, 1
  %6 = icmp sgt i32 %5, %1
  br label %loop.continue

loop.continue:
  call void @bar()
  br i1 %6, label %after.loop, label %loop.start

after.loop:
  %7 = phi i32 [ 0, %2], [ %5, %loop.continue]
  ret i32 %7
}
