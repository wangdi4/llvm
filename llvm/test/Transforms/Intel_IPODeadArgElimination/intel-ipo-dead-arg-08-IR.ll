; RUN: opt  -passes=intel-ipo-dead-arg-elimination -S %s 2>&1 | FileCheck %s

; This test case checks that IPO simplified dead argument elimination does not
; remove argument %0 in function @foo and does not delete the actual parameter
; in function @bas, because @foo is address taken.

; This test case is the same as intel-ipo-dead-arg-08.ll but it checks the IR.

; CHECK: define internal float @foo(ptr %arg, ptr %arg1, i64 %arg2, i64 %arg3)
; CHECK: define internal float @bas(ptr %arg, float %arg1, i64 %arg2, i64 %arg3)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@GV = dso_local global ptr @foo, align 8

define internal float @foo(ptr %arg, ptr %arg1, i64 %arg2, i64 %arg3) {
bb:
  %i = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %arg2, i64 %arg3, ptr nonnull elementtype(float) %arg, i64 %arg2)
  %i4 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %arg2, i64 4, ptr nonnull elementtype(float) %i, i64 %arg2)
  %i5 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %arg2, i64 %arg3, ptr nonnull elementtype(float) %i4, i64 %arg2)
  %i6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %arg2, i64 %arg3, ptr nonnull elementtype(float) %i5, i64 %arg2)
  %i7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %arg2, i64 4, ptr nonnull elementtype(float) %i6, i64 %arg2)
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %arg2, i64 %arg3, ptr nonnull elementtype(float) %i7, i64 %arg2)
  store float 0.000000e+00, ptr %i8, align 4
  %i9 = load float, ptr %arg1, align 4
  ret float %i9
}

define internal float @bas(ptr %arg, float %arg1, i64 %arg2, i64 %arg3) {
bb:
  %t5 = alloca float, i64 %arg3, align 4
  %t6 = call float @foo(ptr %t5, ptr %arg, i64 %arg2, i64 %arg3)
  %t0 = load ptr, ptr @GV, align 8
  %call = call float %t0(ptr %t5, ptr %arg, i64 %arg2, i64 %arg3)
  %t7 = fadd float %arg1, %t6
  %t8 = fadd float %t7, %call
  ret float %t8
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
