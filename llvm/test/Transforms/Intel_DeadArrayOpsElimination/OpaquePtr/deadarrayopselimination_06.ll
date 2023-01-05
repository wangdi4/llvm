; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; This test verifies that transformations for DeadArrayOpsElimination
; optimization is not triggered. "s_qsort" is treated as Qsort function
; since it is marked with "is-qsort". This test is same as
; darrayopselimination_05.ll except "baz" call in "foo". Array Analysis
; unable to detect "array use range info" because the array is escaped
; through "baz" call.
; Note that the functions don't have any valid IR or meaning.

; RUN: opt < %s -S -opaque-pointers -passes='module(deadarrayopselimination)' -debug-only=deadarrayopselimination -disable-output -whole-program-assume 2>&1 | FileCheck %s

; CHECK: DeadArrayOpsElimi: Considering qsort function: s_qsort
; CHECK:  First Call:   call void @s_qsort(ptr nonnull %add.ptr, i64 490)
; CHECK:  Recursion Call:   tail call fastcc void @s_qsort(ptr %t6, i64 %t3)
; CHECK:  ArraySize: 490
; CHECK:  Array Use Info: Full or Empty
; CHECK:  failed: no partial use of array
; CHECK: DeadArrayOpsElimi Failed: No candidates found.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @s_qsort(ptr %arg, i64 %arg1) #0 {
entry:
  %t1 = add i64 %arg1, 1
  %t5 = getelementptr inbounds i8, ptr %arg, i64 1
  br label %bb0

bb0:                                              ; preds = %bb2, %entry
  %t6 = getelementptr inbounds i8, ptr %t5, i64 4
  %t2 = icmp ugt i64 %t1, 8
  br i1 %t2, label %bb1, label %bb2

bb1:                                              ; preds = %bb0
  %t3 = lshr i64 %t1, 3
  tail call fastcc void @s_qsort(ptr %t6, i64 %t3)
  br label %bb2

bb2:                                              ; preds = %bb1, %bb0
  %t4 = icmp ugt i64 %t1, 4
  br i1 %t4, label %bb0, label %bb3

bb3:                                              ; preds = %bb2
  ret void
}

define dso_local void @foo() {
entry:
  %perm = alloca [491 x ptr], align 16
  br label %BB1

BB1:                                              ; preds = %entry
  %add.ptr = getelementptr inbounds ptr, ptr %perm, i64 1
  call void @s_qsort(ptr nonnull %add.ptr, i64 490) #0
  br label %BB2

BB2:                                              ; preds = %BB1
  br label %loop

loop:                                             ; preds = %loop, %BB2
  %iv = phi i64 [ 0, %BB2 ], [ %iv.next, %loop ]
  %iv.next = add i64 %iv, 1
  %ptr = getelementptr inbounds ptr, ptr %perm, i64 %iv
  %p1 = load ptr, ptr %ptr, align 8
  %cmp = icmp eq i64 %iv, 20
  br i1 %cmp, label %exit, label %loop

exit:                                             ; preds = %loop
  call void @baz(ptr %add.ptr)
  ret void
}

define dso_local i32 @main() {
entry:
  call void @foo()
  ret i32 0
}

declare void @baz(ptr)

attributes #0 = { "is-qsort" }
; end INTEL_FEATURE_SW_ADVANCED

