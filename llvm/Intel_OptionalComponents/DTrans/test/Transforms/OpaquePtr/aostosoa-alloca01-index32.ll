; REQUIRES: asserts
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation update of the DTrans metadata attached to
; alloca instructions when using a 32-bit index type.

target datalayout = "e-m:e-i32:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

define void @test01() {
  %ps1 = alloca %struct.test01*, align 8, !intel_dtrans_type !2
  %ps2 = alloca %struct.test01**, align 8, !intel_dtrans_type !3
  %ps3 = alloca %struct.test01dep*, align 8, !intel_dtrans_type !4
  ret void
}
; CHECK-LABEL: define void @test01
; CHECK-NONOPAQUE: %ps1 = alloca i32, align 8
; CHECK-NONOPAQUE-NOT: !intel_dtrans_type
; CHECK-NONOPAQUE: %ps2 = alloca i32*, align 8, !intel_dtrans_type ![[PI32:[0-9]+]]
; CHECK-NONOPAQUE: %ps3 = alloca %__SOADT_struct.test01dep*, align 8, !intel_dtrans_type ![[PDEP:[0-9]+]]

; CHECK-OPAQUE: %ps1 = alloca ptr, align 8
; CHECK-OPAQUE-NOT: !intel_dtrans_type
; CHECK-OPAQUE: %ps2 = alloca ptr, align 8, !intel_dtrans_type ![[PI32:[0-9]+]]
; CHECK-OPAQUE: %ps3 = alloca ptr, align 8, !intel_dtrans_type ![[PDEP:[0-9]+]]

; Test with arrays of pointers
define void @test02() {
  %ps1 = alloca [4 x %struct.test01*], align 8, !intel_dtrans_type !5
  %ps2 = alloca [4 x %struct.test01**], align 8, !intel_dtrans_type !6
  %ps3 = alloca [4 x %struct.test01dep*], align 8, !intel_dtrans_type !7
  ret void
}
; CHECK-NONOPAQUE: %ps1 = alloca [4 x i32], align 8
; CHECK-NONOPAQUE-NOT: !intel_dtrans_type
; CHECK-NONOPAQUE: %ps2 = alloca [4 x i32*], align 8, !intel_dtrans_type ![[API32:[0-9]+]]
; CHECK-NONOPAQUE: %ps3 = alloca [4 x %__SOADT_struct.test01dep*], align 8, !intel_dtrans_type ![[APDEP:[0-9]+]]

; CHECK-OPAQUE: %ps1 = alloca [4 x ptr], align 8
; CHECK-OPAQUE-NOT: !intel_dtrans_type
; CHECK-OPAQUE: %ps2 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[API32:[0-9]+]]
; CHECK-OPAQUE: %ps3 = alloca [4 x ptr], align 8, !intel_dtrans_type ![[APDEP:[0-9]+]]

; CHECK: ![[PI32]] = !{i32 0, i32 1}
; CHECK: ![[PDEP]] = !{%__SOADT_struct.test01dep zeroinitializer, i32 1}
; CHECK: ![[API32]] = !{!"A", i32 4, ![[PI32]]}
; CHECK: ![[APDEP]] = !{!"A", i32 4, ![[PDEP]]}

!intel.dtrans.types = !{!8, !9}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!4 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!5 = !{!"A", i32 4, !2}  ; [4 x %struct.test01*]
!6 = !{!"A", i32 4, !3}  ; [4 x %struct.test01**]
!7 = !{!"A", i32 4, !4}  ; [4 x %struct.test01dep*]
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!9 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }
