; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -passes='module(post-inline-ip-cloning)' -S -ip-cloned-func-arg-merge=false -filter-print-funcs=c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678.1 2>&1 | FileCheck %s

; The test is a copy of ip_cloning_constant_array01.ll but checks IR instead of optimization's
; debug output

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@OFF = global [37 x i64] zeroinitializer
@OFF0 = constant [37 x i64] zeroinitializer

define fastcc void @lbm() {
  unreachable

1:                                                ; No predecessors!
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr null, i32 0, ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, ptr null, ptr @OFF0, i64 0, i64 0, i64 0, i64 0, ptr null, ptr null, i64 0, i64 0)
  br label %3

2:                                                ; No predecessors!
  call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr null, i32 0, ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, ptr null, ptr @OFF, i64 0, i64 0, i64 0, i64 0, ptr null, ptr null, i64 0, i64 0)
  br label %3

3:                                                ; preds = %2, %1
  ret void
}

declare !callback !0 void @__kmpc_fork_call(ptr, i32, ptr, ...)

define internal void @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678(ptr %0, ptr %1, ptr %2, ptr %3, i64 %4, i64 %5, i64 %6, i64 %7, ptr %8, ptr %9, i64 %10, i64 %11) {
  br label %13

13:                                               ; preds = %13, %12
  %14 = getelementptr i64, ptr %3, i64 0
  %15 = load i64, ptr %14, align 8
  %16 = getelementptr double, ptr %9, i64 %15
  %17 = load double, ptr %16, align 8
  br label %13
}

; uselistorder directives
uselistorder ptr @__kmpc_fork_call, { 1, 0 }
uselistorder ptr @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678, { 1, 0 }

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}

; CHECK-LABEL: define internal void @c_kernel.DIR.OMP.PARALLEL.LOOP.2.split678.1(
; CHECK-SAME: ptr [[TMP0:%.*]], ptr [[TMP1:%.*]], ptr [[TMP2:%.*]], ptr [[TMP3:%.*]], i64 [[TMP4:%.*]], i64 [[TMP5:%.*]], i64 [[TMP6:%.*]], i64 [[TMP7:%.*]], ptr [[TMP8:%.*]], ptr [[TMP9:%.*]], i64 [[TMP10:%.*]], i64 [[TMP11:%.*]]) {
; CHECK-NEXT:    br label [[TMP13:%.*]]
; CHECK:       13:
; CHECK-NEXT:    [[TMP14:%.*]] = getelementptr i64, ptr [[TMP3]], i64 0
; CHECK-NEXT:    [[TMP15:%.*]] = load i64, ptr [[TMP14]], align 8
; CHECK-NEXT:    [[TMP16:%.*]] = getelementptr double, ptr [[TMP9]], i64 0
; CHECK-NEXT:    [[TMP17:%.*]] = load double, ptr [[TMP16]], align 8
; CHECK-NEXT:    br label [[TMP13]]
;
; end INTEL_FEATURE_SW_ADVANCED
