; REQUIRES: asserts

; RUN: opt < %s -passes='module(intel-loop-attrs)' -force-intel-must-progress -debug-only=intel-loop-attrs -disable-output 2>&1 | FileCheck %s

; This test case checks that all the loops in @foo weren't marked
; "mustprogress" since they are nested loops. Function @foo should
; not be marked "mustprogress".

; CHECK: Loop "mustprogress" results for function foo:
; CHECK-NEXT:    Result for function foo, loop in BB loop.start: "mustprogress" loop attribute NOT added
; CHECK-NEXT:    Result for function foo, loop in BB nested.loop.start: "mustprogress" loop attribute NO
; CHECK-NOT: "mustprogress" function attribute added to function foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %0, i32 %1, i32 %2) {
3:
  %4 = icmp sgt i32 %0, %2
  br i1 %4, label %exit.block, label %loop.start

loop.start:
  %5 = phi i32 [ 0, %3], [ %6, %nested.loop.start]
  %6 = add nuw nsw i32 %5, 1
  %7 = icmp sgt i32 %6, %2
  br i1 %7, label %exit.block, label %nested.loop.start

nested.loop.start:
  %8 = phi i32 [ 0, %loop.start], [ %9, %nested.loop.start]
  %9 = add nuw nsw i32 %8, 1
  %10 = icmp sgt i32 %9, %1
  br i1 %10, label %loop.start, label %nested.loop.start

exit.block:
  %11 = phi i32 [ 0, %3], [ %6, %loop.start]
  ret i32 %11
}
