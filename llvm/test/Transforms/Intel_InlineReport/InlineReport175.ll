; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; This test verifies that the inlining reports for DeadArrayOpsElimination
; optimization are done correctly. "-dead-array-ops-functions" is used to
; indicate s_qsort is Qsort function and used high index for %perm is 60.
; Note that the functions don't have any valid IR or meaning.

; RUN: opt -passes='module(deadarrayopselimination),print<inline-report>' -dead-array-ops-functions="s_qsort,60" -whole-program-assume -inline-report=0xf847 -disable-output < %s 2>&1 | FileCheck --check-prefix=CHECK-CL %s
; RUN: opt -passes='inlinereportsetup,module(deadarrayopselimination),inlinereportemitter' -dead-array-ops-functions="s_qsort,60" -whole-program-assume -inline-report=0xf8c6 -disable-output < %s 2>&1 | FileCheck --check-prefix=CHECK-MD %s

; CHECK-CL: DEAD STATIC FUNC: s_qsort
; CHECK-CL: COMPILE FUNC: s_qsort.1
; CHECK-CL: s_qsort.1
; CHECK-CL: COMPILE FUNC: foo
; CHECK-CL: s_qsort.1
; CHECK-CL: COMPILE FUNC: main
; CHECK-CL: foo

; CHECK-MD: DEAD STATIC FUNC: s_qsort
; CHECK-MD: COMPILE FUNC: foo
; CHECK-MD: s_qsort.1
; CHECK-MD: COMPILE FUNC: main
; CHECK-MD: foo
; CHECK-MD: COMPILE FUNC: s_qsort.1
; CHECK-MD: s_qsort.1

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
  %add.ptr = getelementptr inbounds ptr, ptr %perm, i64 1
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
