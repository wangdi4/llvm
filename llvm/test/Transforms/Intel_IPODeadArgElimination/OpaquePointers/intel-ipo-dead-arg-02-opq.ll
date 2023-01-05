; REQUIRES: asserts

; RUN: opt -opaque-pointers -passes=intel-ipo-dead-arg-elimination -debug-only=intel-ipo-dead-arg-elimination  %s -disable-output 2>&1 | FileCheck %s -check-prefix=CHECK-FOO
; RUN: opt -opaque-pointers -passes=intel-ipo-dead-arg-elimination -debug-only=intel-ipo-dead-arg-elimination  %s -disable-output 2>&1 | FileCheck %s -check-prefix=CHECK-BAR

; This test case checks that IPO simplified dead argument elimination removes
; argument %0 in functions @foo and @bar, and deletes the actual parameter
; in function @bas. This is the same test case as intel-ipo-dead-arg-02.ll but
; it checks for opaque pointers.

; CHECK: Debug information for IPO dead arg elimination:
; CHECK-NEXT:   Candidates collected: 2
; CHECK-BAR:          Function: bar
; CHECK-BAR-NEXT:       Arg[0]: ptr %0

; CHECK-FOO:          Function: foo
; CHECK-FOO-NEXT:       Arg[0]: ptr %0

; CHECK:   Candidates after analysis: 2
; CHECK-BAR:         Function: bar
; CHECK-BAR-NEXT:       Arg[0]: ptr %0

; CHECK-FOO:         Function: foo
; CHECK-FOO-NEXT:       Arg[0]: ptr %0

; CHECK:   Functions transformed:
; CHECK-BAR:         Function: bar
; CHECK-BAR-NEXT:         Old number of arguments: 4
; CHECK-BAR-NEXT:         New number of arguments: 3

; CHECK-FOO:          Function: foo
; CHECK-FOO-NEXT:         Old number of arguments: 4
; CHECK-FOO-NEXT:         New number of arguments: 3

; CHECK:     Total functions modified: 2

; CHECK:   Actual parameters removed:
; CHECK-NEXT:     Function: bas
; CHECK-NEXT:       Instruction:   %5 = alloca float, i64 %3, align 4
; CHECK-NEXT:    Total of actual parameter removed: 1

; ModuleID = 'intel-ipo-dead-arg-02.ll'
source_filename = "intel-ipo-dead-arg-02.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal float @foo(ptr %0, ptr %1, i64 %2, i64 %3) {
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

define internal float @bar(ptr %0, ptr %1, i64 %2, i64 %3) {
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

define internal float @bas(ptr %0, ptr %1, i64 %2, i64 %3) {
  %5 = alloca float, i64 %3, align 4
  %6 = call float @foo(ptr %5, ptr %0, i64 %2, i64 %3)
  %7 = call float @bar(ptr %5, ptr %1, i64 %2, i64 %3)
  %8 = fadd float %6, %7
  ret float %8
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }
