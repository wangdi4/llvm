; RUN: opt -intel-ipo-dead-arg-elimination -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 %s -S 2>&1 | FileCheck %s
; RUN: opt -passes=intel-ipo-dead-arg-elimination -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 %s -S 2>&1 | FileCheck %s

; This test case checks that IPO simplified dead argument elimination won't
; remove argument %0 in function @foo since the actual parameter is passed
; to @bar, which isn't a candidate for the transformation. This test case is
; the same as intel-ipo-dead-arg-03.ll but it checks the IR.

; CHECK: define internal float @foo(float* %0, float* %1, i64 %2, i64 %3) #1 {
; CHECK-NEXT:   %5 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* nonnull elementtype(float) %0, i64 %2)
; CHECK-NEXT:   %6 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* nonnull elementtype(float) %5, i64 %2)
; CHECK-NEXT:   %7 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* nonnull elementtype(float) %6, i64 %2)
; CHECK-NEXT:   %8 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 %2, i64 %3, float* nonnull elementtype(float) %7, i64 %2)
; CHECK-NEXT:   %9 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 %2, i64 4, float* nonnull elementtype(float) %8, i64 %2)
; CHECK-NEXT:   %10 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 %2, i64 %3, float* nonnull elementtype(float) %9, i64 %2)
; CHECK-NEXT:   store float 0.000000e+00, float* %10, align 4
; CHECK-NEXT:   %11 = load float, float* %1, align 4
; CHECK-NEXT:   ret float %11
; CHECK-NEXT: }

; CHECK: define internal float @bas(float* %0, float* %1, i64 %2, i64 %3) #1 {
; CHECK-NEXT:   %5 = alloca float, i64 %3, align 4
; CHECK-NEXT:   %6 = call float @foo(float* %5, float* %0, i64 %2, i64 %3)
; CHECK-NEXT:   %7 = call float @bar(float* %5, float* %1, i64 %2, i64 %3)
; CHECK-NEXT:   %8 = fadd float %6, %7
; CHECK-NEXT:   ret float %8
; CHECK-NEXT: }

; CHECK: attributes #1 = { "target-features"="+avx2" }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 %0, i64 %1, i64 %2, float* elementtype(float) %3, i64 %4)

define internal float @foo(float *%0, float *%1, i64 %2, i64 %3) {
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

declare float @bar(float *%0, float *%1, i64 %2, i64 %3)

define internal float @bas(float *%0, float *%1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3
  %6 = call float @foo(float *%5, float *%0, i64 %2, i64 %3)
  %7 = call float @bar(float *%5, float *%1, i64 %2, i64 %3)
  %8 = fadd float %6, %7
  ret float %8
}