; This test verifies that first operand values of %p1 and %p2 PHINodes are
; propagated to the uses of %p1 and %p2 in "bb_action" BasicBlock.

; RUN: opt < %s -S -jump-threading | FileCheck %s
; RUN: opt < %s -S -passes='function(jump-threading)' | FileCheck %s

; CHECK: %cond1 = icmp eq i8* %p2, null
; CHECK: %cond2 = icmp eq i8* %p1, null
; CHECK: %or.cond = or i1 %cond1, %cond2
; CHECK: br i1 %or.cond, label %bb_cleanupret, label %bb_action
; CHECK: bb_action:
; CHECK: %fptr = bitcast i8* bitcast (void (i8*)* @Func to i8*) to void (i8*)*
; CHECK: call void %fptr(i8* %0)

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

define void @foo(i8* %0) personality i8* bitcast (i32 ()* @baz to i8*) {
bb_0:
  invoke void @bar()
          to label %bb_1 unwind label %bb_cleanuppad

bb_1:
  invoke void @bar()
          to label %bb_exit unwind label %bb_cleanuppad

bb_cleanuppad:
  %p1 = phi i8* [bitcast (void (i8*)* @"Func" to i8*), %bb_1 ], [ null, %bb_0 ]
  %p2 = phi i8* [ %0, %bb_1 ], [ null, %bb_0 ]
  %1 = cleanuppad within none []
  %cond1 = icmp eq i8* %p2, null
  %cond2 = icmp eq i8* %p1, null
  %or.cond = or i1 %cond1, %cond2
  br i1 %or.cond, label %bb_cleanupret, label %bb_action

bb_action:
  %fptr = bitcast i8* %p1 to void (i8*)*
  call void %fptr(i8* %p2)
  br label %bb_exit

bb_exit:
  call void @bar()
  ret void

bb_cleanupret:
  cleanupret from %1 unwind to caller
}

declare void @Func(i8*)
declare void @bar()
declare i32 @baz()
