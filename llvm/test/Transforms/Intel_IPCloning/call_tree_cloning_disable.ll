; RUN: opt < %s -passes='module(call-tree-clone)' -call-tree-clone-disable=true -S | FileCheck %s

; Check that -call-tree-clone-disable knob can disable Call Tree Cloning

; CHECK-LABEL: @foo(
; CHECK-NOT: "foo|

source_filename = "call_tree_cloning_disable.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.1 = private unnamed_addr constant [9 x i8] c"foo(%p)\0A\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"loop:%d\0A\00", align 1
@a = internal global [100 x float] zeroinitializer, align 16

define internal fastcc void @foo(ptr %0, i32 %1) {
  %3 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) getelementptr inbounds ([9 x i8], ptr @.str.1, i64 0, i64 0), ptr %0)
  br label %4

4:                                                ; preds = %4, %2
  %5 = phi i32 [ %7, %4 ], [ 0, %2 ]
  %6 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) getelementptr inbounds ([9 x i8], ptr @.str.2, i64 0, i64 0), i32 %5)
  %7 = add nuw nsw i32 %5, 1
  %8 = icmp eq i32 %7, %1
  br i1 %8, label %9, label %4

9:                                                ; preds = %4
  ret void
}

define i32 @main() {
  tail call fastcc void @foo(ptr getelementptr inbounds ([100 x float], ptr @a, i64 0, i64 0), i32 10)
  ret i32 0
}

declare i32 @printf(ptr nocapture readonly, ...)
