; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output -debug-only=dtrans-safetyanalyzer %s 2>&1 | FileCheck %s

; This test is to verify that a crash does not occur due to having
; invalid metadata. (CMPLRLLVM-36519). Instead, the safety analyzer
; should bail out because reading the metadata was not successful.

; CHECK: DTransSafetyInfo: Type metadata reader did not find structure type metadata

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.a = type { i32, i32 }

define dso_local void @test(ptr noundef "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !4 {
entry:
  %0 = getelementptr inbounds %struct.test.a, ptr %ptr, i64 0, i32 1
  ret void
}

!intel.dtrans.types = !{!1}

!0 = !{i32 0, i32 0}
; This structure desciptor is invalid metadata, because a field type refers back
; to the structure being defined, so does not match the actual type of the LLVM
; structure.
!1 = !{!"S", %struct.test.a zeroinitializer, i32 2, !1, !1} 
!2 = !{%struct.test.a zeroinitializer, i32 0}
!3 = !{%struct.test.a zeroinitializer, i32 1}
!4 = distinct !{!3}
