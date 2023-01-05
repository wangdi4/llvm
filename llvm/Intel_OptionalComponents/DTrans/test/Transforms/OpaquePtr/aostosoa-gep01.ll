; REQUIRES: asserts
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation for GEP instructions

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, i32 }

; CHECK-DAG: %__SOA_struct.test01 = type { ptr, ptr, ptr }
; CHECK-DAG: %__SOADT_struct.test01dep = type { i64, i32 }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0
  %p2 = getelementptr %struct.test01, ptr %p1, i64 0, i32 2
  %v = load i32, ptr %p2
  ret i32 0
}
; CHECK-LABEL: define i32 @test01

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %p1 = load i64, ptr %p0
; CHECK: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-SAME: !invariant.load
; CHECK: %p2 = getelementptr i32, ptr %[[LOAD1]], i64 %p1
; CHECK: %v = load i32, ptr %p2
; CHECK: ret i32 0

; Use a single index GEP on the type being transformed.
define i32 @test02(i64 %n) {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0
  %p2 = getelementptr %struct.test01, ptr %p1, i64 %n
  %p3 = getelementptr %struct.test01, ptr %p2, i64 0, i32 2
  %v = load i32, ptr %p3
  ret i32 0
}

; CHECK-LABEL: define i32 @test02

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %p1 = load i64, ptr %p0
; CHECK: %p2 = add i64 %p1, %n
; CHECK: %[[GEP1:[0-9]+]]  = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-SAME: !invariant.load
; CHECK: %p3 = getelementptr i32, ptr %[[LOAD1]], i64 %p2
; CHECK: %v = load i32, ptr %p3
; CHECK: ret i32 0

; Use a variable for the first GEP index on the type being transformed.
define i32 @test03(i64 %n) {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0
  %p2 = getelementptr %struct.test01, ptr %p1, i64 %n, i32 2
  %v = load i32, ptr %p2
  ret i32 0
}

; CHECK-LABEL: define i32 @test03


; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %p1 = load i64, ptr %p0
; CHECK: %[[GEP1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[GEP1]]
; CHECK-SAME: !invariant.load
; CHECK: %[[ADD1:[0-9]+]] = add i64 %p1, %n
; CHECK: %p2 = getelementptr i32, ptr %[[LOAD1]], i64 %[[ADD1]]
; CHECK: %v = load i32, ptr %p2
; CHECK: ret i32 0


!intel.dtrans.types = !{!3, !4}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !1} ; { %struct.test01*, i32 }

