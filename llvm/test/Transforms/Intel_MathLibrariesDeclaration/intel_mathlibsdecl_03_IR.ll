; RUN: opt -passes=intel-math-libraries-decl %s -S 2>&1 | FileCheck %s

; This test case checks that the declaration for exp (long) was
; generated correctly since llvm.exp.f128 is in the module. This is
; the same test case as intel_mathlibsdecl_01.ll, but it checks the
; IR.

; CHECK: declare fp128 @expl(fp128) #0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define fp128 @foo(fp128 %0) {
entry:
  %1 = call fast fp128 @llvm.exp.f128(fp128 %0)
  ret fp128 %1
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn mustprogress
declare fp128 @llvm.exp.f128(fp128 %0) #0

attributes #0 = { nofree nosync nounwind readnone speculatable willreturn }
