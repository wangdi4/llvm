; REQUIRES: asserts
; RUN: opt -passes=intel-math-libraries-decl -debug-only=intel-math-libraries-decl %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the declaration for exp (float) was
; generated correctly since llvm.exp.f32 is in the module.

; CHECK: Math function declarations added:
; CHECK:   Intrinsic @llvm.exp.f32 generates function @expf
; CHECK:   Total functions added: 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(float %0) {
entry:
  %1 = call fast float @llvm.exp.f32(float %0)
  ret float %1
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn mustprogress
declare float @llvm.exp.f32(float %0) #0

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
