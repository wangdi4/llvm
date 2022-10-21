; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation conversion of alloca instructions and
; the update of the DTrans metadata attached to them.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

define void @test01() {
  %ps1 = alloca %struct.test01*, align 8, !intel_dtrans_type !2
  %ps2 = alloca %struct.test01**, align 8, !intel_dtrans_type !3
  %ps3 = alloca %struct.test01dep*, align 8, !intel_dtrans_type !4

  ; Use the allocation that will be converted to an integer type to
  ; verify the users are updated.
  %use1 = getelementptr %struct.test01*, %struct.test01** %ps1, i64 0
  store %struct.test01* null, %struct.test01** %ps1
  %load = load %struct.test01*, %struct.test01** %ps1
  ret void
}
; CHECK-LABEL: define void @test01
; CHECK-NONOPAQUE: %ps1 = alloca i64, align 8
; CHECK-NONOPAQUE-NOT: !intel_dtrans_type
; CHECK-NONOPAQUE: %ps2 = alloca i64*, align 8, !intel_dtrans_type ![[PI64:[0-9]+]]
; CHECK-NONOPAQUE: %ps3 = alloca %__SOADT_struct.test01dep*, align 8, !intel_dtrans_type ![[PDEP:[0-9]+]]
; CHECK-NONOPAQUE: %use1 = getelementptr i64, i64* %ps1, i64 0
; CHECK-NONOPAQUE: store i64 0, i64* %ps1
; CHECK-NONOPAQUE: %load = load i64, i64* %ps1

; CHECK-OPAQUE: %ps1 = alloca i64, align 8
; CHECK-OPAQUE-NOT: !intel_dtrans_type
; CHECK-OPAQUE: %ps2 = alloca ptr, align 8, !intel_dtrans_type ![[PI64:[0-9]+]]
; CHECK-OPAQUE: %ps3 = alloca ptr, align 8, !intel_dtrans_type ![[PDEP:[0-9]+]]
; CHECK-OPAQUE: %use1 = getelementptr ptr, ptr %ps1, i64 0
; CHECK-OPAQUE: store i64 0, ptr %ps1
; CHECK-OPAQUE: %load = load i64, ptr %ps1

; Test with arrays of pointers
define void @test02() {
  %ps2 = alloca [4 x %struct.test01**], align 8, !intel_dtrans_type !5
  %ps3 = alloca [4 x %struct.test01dep*], align 8, !intel_dtrans_type !6
  ret void
}
; CHECK-NONOPAQUE: %ps2 = alloca [4 x i64*], align 8, !intel_dtrans_type ![[API64:[0-9]+]]
; CHECK-NONOPAQUE: %ps3 = alloca [4 x %__SOADT_struct.test01dep*], align 8, !intel_dtrans_type ![[APDEP:[0-9]+]]

; CHECK-OPAQUE: %ps2 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[API64:[0-9]+]]
; CHECK-OPAQUE: %ps3 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[APDEP:[0-9]+]]

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
