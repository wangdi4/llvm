; REQUIRES: asserts
; RUN: opt -passes=intel-math-libraries-decl -debug-only=intel-math-libraries-decl %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the declaration for exp (double) was
; generated correctly since llvm.exp.f64 is in the module.

; CHECK: Math function declarations added:
; CHECK:   Intrinsic @llvm.exp.f64 generates function @exp
; CHECK:   Total functions added: 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @foo(double %0) {
entry:
  %1 = call fast double @llvm.exp.f64(double %0)
  ret double %1
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn mustprogress
declare double @llvm.exp.f64(double %0) #0

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
