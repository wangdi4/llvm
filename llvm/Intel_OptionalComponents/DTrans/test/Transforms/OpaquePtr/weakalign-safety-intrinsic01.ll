; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

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
define internal  "intel_dtrans_func_index"="1" ptr @test01(ptr "intel_dtrans_func_index"="2" %x) !intel.dtrans.func.type !3 {
  %ptr = load ptr, ptr %x, align 8
  %ptrint = ptrtoint ptr %ptr to i64
  %maskedptr = and i64 %ptrint, 63
  %maskcond = icmp eq i64 %maskedptr, 0
  tail call void @llvm.assume(i1 %maskcond)
  ret ptr %ptr
}

define i32 @main() {
  %d = alloca ptr, align 64, !intel_dtrans_type !1
  %d2 = call ptr @test01(ptr %d)
  ret i32 0
}

declare void @llvm.assume(i1)

!1 = !{double 0.0e+00, i32 1}  ; double*
!2 = !{double 0.0e+00, i32 2}  ; double**
!3 = distinct !{!1, !2}

!intel.dtrans.types = !{}

