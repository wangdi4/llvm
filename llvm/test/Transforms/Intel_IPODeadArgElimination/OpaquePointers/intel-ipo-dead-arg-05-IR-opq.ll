; RUN: opt -opaque-pointers -passes=intel-ipo-dead-arg-elimination  %s -S 2>&1 | FileCheck %s

; This test case checks that IPO simplified dead argument elimination won't
; be performed since function @foo is set as naked. This is the same
; test case as intel-ipo-dead-arg-05-IR.ll but it checks the IR with opaque
; pointers.

; CHECK: define internal float @foo(ptr %0, ptr %1, i64 %2, i64 %3) #0 {
; CHECK-NEXT:   %5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %0, i64 %2)
; CHECK-NEXT:   %6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %5, i64 %2)
; CHECK-NEXT:   %7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %6, i64 %2)
; CHECK-NEXT:   %8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %7, i64 %2)
; CHECK-NEXT:   %9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %8, i64 %2)
; CHECK-NEXT:   %10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %9, i64 %2)
; CHECK-NEXT:   store float 0.000000e+00, ptr %10, align 4
; CHECK-NEXT:   %11 = load float, ptr %1, align 4
; CHECK-NEXT:   ret float %11

; CHECK: define float @bas(ptr %0, float %1, i64 %2, i64 %3)
; CHECK-NEXT:   %5 = alloca float, i64 %3, align 4
; CHECK-NEXT:   %6 = call float @foo(ptr %5, ptr %0, i64 %2, i64 %3)
; CHECK-NEXT:   %7 = fadd float %1, %6
; CHECK-NEXT:   ret float %7

; CHECK: attributes #0 = { naked }

; ModuleID = 'intel-ipo-dead-arg-05-IR.ll'
source_filename = "intel-ipo-dead-arg-05-IR.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: naked
define internal float @foo(ptr %0, ptr %1, i64 %2, i64 %3) #0 {
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %0, i64 %2)
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %5, i64 %2)
  %7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %6, i64 %2)
  %8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %2, i64 %3, ptr nonnull elementtype(float) %7, i64 %2)
  %9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %2, i64 4, ptr nonnull elementtype(float) %8, i64 %2)
  %10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %2, i64 %3, ptr nonnull elementtype(float) %9, i64 %2)
  store float 0.000000e+00, ptr %10, align 4
  %11 = load float, ptr %1, align 4
  ret float %11
}

define float @bas(ptr %0, float %1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3, align 4
  %6 = call float @foo(ptr %5, ptr %0, i64 %2, i64 %3)
  %7 = fadd float %1, %6
  ret float %7
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { naked }
attributes #1 = { nounwind readnone speculatable }
