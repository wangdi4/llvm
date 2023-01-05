; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; This test verifies that DeadArrayOpsElimination optimization is not triggered
; for s_qsort even though "dead-array-ops-functions" is used to indicate
; s_qsort is Qsort function and used high index for %perm is 60 since s_qsort
; has more than 2 calls.
; Note that the functions don't have any valid IR or meaning.

; RUN: opt < %s  -opaque-pointers -passes='module(deadarrayopselimination)' -dead-array-ops-functions="s_qsort,60" -debug-only=deadarrayopselimination -whole-program-assume -disable-output 2>&1 | FileCheck %s

; CHECK: DeadArrayOpsElimi: Considering qsort function: s_qsort
; CHECK: failed: sanity checks
; CHECK: DeadArrayOpsElimi Failed: No candidates found

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @s_qsort(ptr %arg, i64 %arg1) {
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
  %add.ptr = getelementptr inbounds i8, ptr %perm, i64 1
  call void @s_qsort(ptr nonnull %add.ptr, i64 490)
  call void @s_qsort(ptr nonnull %add.ptr, i64 490)
  br label %BB2

BB2:                                              ; preds = %BB1
  ret void
}

define dso_local i32 @main() {
entry:
  call void @foo()
  ret i32 0
}
; end INTEL_FEATURE_SW_ADVANCED
