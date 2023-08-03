; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation conversion of alloca instructions and
; the update of the DTrans metadata attached to them.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, ptr }

define void @test01() {
  %ps1 = alloca ptr, align 8, !intel_dtrans_type !2
  %ps2 = alloca ptr, align 8, !intel_dtrans_type !3
  %ps3 = alloca ptr, align 8, !intel_dtrans_type !4

  ; Use the allocation that will be converted to an integer type to
  ; verify the users are updated.
  %use1 = getelementptr ptr, ptr %ps1, i64 0
  store ptr null, ptr %ps1
  %load = load ptr, ptr %ps1
  ret void
}
; CHECK-LABEL: define void @test01

; CHECK: %ps1 = alloca i64, align 8
; CHECK-NOT: !intel_dtrans_type
; CHECK: %ps2 = alloca ptr, align 8, !intel_dtrans_type ![[PI64:[0-9]+]]
; CHECK: %ps3 = alloca ptr, align 8, !intel_dtrans_type ![[PDEP:[0-9]+]]
; CHECK: %use1 = getelementptr ptr, ptr %ps1, i64 0
; CHECK: store i64 0, ptr %ps1
; CHECK: %load = load i64, ptr %ps1

; Test with arrays of pointers
define void @test02() {
  %ps2 = alloca [4 x ptr], align 8, !intel_dtrans_type !5
  %ps3 = alloca [4 x ptr], align 8, !intel_dtrans_type !6
  ret void
}

; CHECK: %ps2 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[API64:[0-9]+]]
; CHECK: %ps3 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[APDEP:[0-9]+]]

; CHECK: ![[PI64]] = !{i64 0, i32 1}
; CHECK: ![[PDEP]] = !{%__SOADT_struct.test01dep zeroinitializer, i32 1}
; CHECK: ![[API64]] = !{!"A", i32 4, ![[PI64]]}
; CHECK: ![[APDEP]] = !{!"A", i32 4, ![[PDEP]]}

!intel.dtrans.types = !{!7, !8}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!4 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!5 = !{!"A", i32 4, !3}  ; [4 x %struct.test01**]
!6 = !{!"A", i32 4, !4}  ; [4 x %struct.test01dep*]
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!8 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }
