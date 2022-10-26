; RUN: opt < %s -passes='module(intel-loop-attrs)' -S 2>&1 | FileCheck %s

; This test case checks that function @foo is candidate for adding the loop
; attributes without using "-force-intel-must-progress" since there is a use
; for it (@main call @foo). This is the same test case as intel_loopattrs07.ll
; but it checks the IR.

; CHECK: define i32 @foo(i32 %0, i32 %1) #0 {
; CHECK:   br i1 %6, label %after.loop, label %loop.start, !llvm.loop !0

; CHECK: define i32 @main() {

; CHECK: attributes #0 = { mustprogress }

; CHECK: !0 = distinct !{!0, !1}
; CHECK: !1 = !{!"llvm.loop.mustprogress"}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %0, i32 %1) {
2:
  %3 = icmp sgt i32 %0, %1
  br i1 %3, label %after.loop, label %loop.start

loop.start:
  %4 = phi i32 [ 0, %2], [ %5, %loop.start]
  %5 = add nuw nsw i32 %4, 1
  %6 = icmp sgt i32 %5, 10
  br i1 %6, label %after.loop, label %loop.start

after.loop:
  %7 = phi i32 [ 0, %2], [ %5, %loop.start]
  ret i32 %7
}

define i32 @main() {
0:
  %1 = alloca i32
  store i32 0, i32* %1, align 4
  %2 = load i32, i32* %1
  %3 = call i32 @foo(i32 %2, i32 10)
  ret i32 %3
}
