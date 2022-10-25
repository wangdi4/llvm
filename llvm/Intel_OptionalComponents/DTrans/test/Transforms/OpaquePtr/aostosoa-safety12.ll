; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -debug-only=dtrans-aostosoaop %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation rejects structures
; that have arrays of pointers to the type. These are rejected because
; otherwise instructions using the alloca need to be modified to
; use an array of the peeling index type, which may have a different
; size than a pointer.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

define void @test01() {
  %ps1 = alloca [4 x %struct.test01*], align 8, !intel_dtrans_type !3
  %use = getelementptr [4 x %struct.test01*], [4 x %struct.test01*]* %ps1, i64 1
  ret void
}

; CHECK: AOS-to-SOA rejecting -- Array of type seen: %struct.test01

!intel.dtrans.types = !{!4, !5}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"A", i32 4, !2}  ; [4 x %struct.test01*]
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }
