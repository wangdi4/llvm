; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation of storing a pointer to the type being
; transformed by AOS-to-SOA.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, ptr }

; Check the transformation when the field is within a dependent type.
@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %v0 = load ptr, ptr %p0
  %v1 = load ptr, ptr %p1
  store ptr %v0, ptr %p1
  store ptr %v1, ptr %p0
  ret i32 0
}

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %v0 = load i64, ptr %p0
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %v1 = load i64, ptr %p1
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 %v0, ptr %p1
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 %v1, ptr %p0
; CHECK: ret i32 0

; Check the transformation when the field is a member of the type being transformed.
define i32 @test02() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %v0 = load ptr, ptr %p0

  %t0 = getelementptr %struct.test01, ptr %v0, i64 0, i32 1
  store ptr %v0, ptr %t0
  ret i32 0
}
; CHECK-LABEL: define i32 @test02

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %v0 = load i64, ptr %p0
; CHECK: [[ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: [[BASE1:%[0-9]+]] = load ptr, ptr [[ADDR1]]
; CHECK-SAME: !invariant.load
; CHECK: %t0 = getelementptr i64, ptr [[BASE1]], i64 %v0
; CHECK: call ptr @llvm.ptr.annotation.p0.p0(ptr %t0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 %v0, ptr %t0
; CHECK: ret i32 0

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
