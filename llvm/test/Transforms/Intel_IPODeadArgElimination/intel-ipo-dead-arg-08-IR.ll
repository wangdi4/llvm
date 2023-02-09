; RUN: opt -opaque-pointers=0 -passes=intel-ipo-dead-arg-elimination %s -S 2>&1 | FileCheck %s

; This test case checks that IPO simplified dead argument elimination does not
; remove argument %0 in function @foo and does not delete the actual parameter
; in function @bas, because @foo is address taken.

; This test case is the same as intel-ipo-dead-arg-08.ll but it checks the IR.

; CHECK: define internal float @foo(float* %0, float* %1, i64 %2, i64 %3)
; CHECK: define internal float @bas(float* %0, float %1, i64 %2, i64 %3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@GV = dso_local global float (float *, float *, i64, i64)* @foo, align 8

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4)

define internal float @foo(float* %0, float* %1, i64 %2, i64 %3) {
   %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* elementtype(float) nonnull %0, i64 %2)
   %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* elementtype(float) nonnull %5, i64 %2)
   %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* elementtype(float) nonnull %6, i64 %2)
   %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* elementtype(float) nonnull %7, i64 %2)
   %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* elementtype(float) nonnull %8, i64 %2)
   %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* elementtype(float) nonnull %9, i64 %2)
   store float 0.000000e+00, float* %10
   %11 = load float, float* %1
   ret float %11
}

define internal float @bas(float* %0, float %1, i64 %2, i64 %3) {
  %t5 = alloca float, i64 %3
  %t6 = call float @foo(float *%t5, float *%0, i64 %2, i64 %3)
  %t0 = load float (float *, float *, i64, i64)*, float (float *, float *, i64, i64)** @GV, align 8
  %call = call float %t0(float *%t5, float *%0, i64 %2, i64 %3)
  %t7 = fadd float %1, %t6
  %t8 = fadd float %t7, %call
  ret float %t8
}
