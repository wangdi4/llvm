; REQUIRES: asserts
; RUN: opt -passes=intel-ipo-dead-arg-elimination -debug-only=intel-ipo-dead-arg-elimination  %s -disable-output 2>&1 | FileCheck %s

; This test case checks that IPO simplified dead argument elimination removes
; argument %0 in function @foo, and deletes the actual parameter in
; function @bas.

; CHECK: Debug information for IPO dead arg elimination:
; CHECK-NEXT:   Candidates collected: 1
; CHECK-NEXT:     Function: foo
; CHECK-NEXT:       Arg[0]: float* %0

; CHECK:   Candidates after analysis: 1
; CHECK-NEXT:     Function: foo
; CHECK-NEXT:       Arg[0]: float* %0

; CHECK:   Functions transformed:

; CHECK:     Function: foo
; CHECK-NEXT:         Old number of arguments: 4
; CHECK-NEXT:         New number of arguments: 3

; CHECK:     Total functions modified: 1

; CHECK:   Actual parameters removed:
; CHECK-NEXT:     Function: bas
; CHECK-NEXT:       Instruction:   %5 = alloca float, i64 %3, align 4
; CHECK-NEXT:    Total of actual parameter removed: 1

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

define internal float @bas(float *%0, float %1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3
  %6 = call float @foo(float *%5, float *%0, i64 %2, i64 %3)
  %7 = fadd float %1, %6
  ret float %7
}
