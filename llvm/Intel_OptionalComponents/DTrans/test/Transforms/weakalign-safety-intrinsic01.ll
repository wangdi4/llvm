; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; assume intrinsics.

; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Source file defined *x using an align_value attribute as follows:
; typedef double * __attribute__((align_value(64))) aligned_double
;
; This leads to llvm.assume intrinsic. In this test, the variable
; is coming from the stack, so weak align transform will not affect
; the variable, but the transform is conservatively disabled for
; any case of the assume intrinsic to avoid the need to trace the
; variable back to its source definition.
define internal double* @test01(double** %x) {
  %ptr = load double*, double** %x, align 8
  %ptrint = ptrtoint double* %ptr to i64
  %maskedptr = and i64 %ptrint, 63
  %maskcond = icmp eq i64 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  ret double* %ptr
}

define i32 @main() {
  %d = alloca double*, align 64
  %d2 = call double* @test01(double** %d)
  ret i32 0
}

declare void @llvm.assume(i1)
