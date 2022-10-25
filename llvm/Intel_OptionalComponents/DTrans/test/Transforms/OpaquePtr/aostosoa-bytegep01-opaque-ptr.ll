; REQUIRES: asserts
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true  -debug-only=dtrans-aostosoaop %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation for byte-flattened GEP instructions

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, i32 }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = load ptr, ptr %p0

  %fa0 = getelementptr i8, ptr %p1, i64 0
  %v0 = load i32, ptr %fa0
  store i32 0, ptr %fa0

  %fa1 = getelementptr i8, ptr %p1, i64 8
  %v1 = load ptr, ptr %fa1
  store ptr null, ptr %fa1

  %fa2 = getelementptr i8, ptr %p1, i64 16
  %v2 = load i32, ptr %fa2
  store i32 0, ptr %fa2

  ret i32 0
}
; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0(ptr %p0, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %p1 = load i64, ptr %p0
; CHECK: %[[FADDR0:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 0
; CHECK: %[[LOAD0:[0-9]+]] = load ptr, ptr %[[FADDR0]]
; CHECK: %fa0 = getelementptr i32, ptr %[[LOAD0]], i64 %p1
; CHECK: %v0 = load i32, ptr %fa0
; CHECK: store i32 0, ptr %fa0
; CHECK: %[[FADDR1:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 1
; CHECK: %[[LOAD1:[0-9]+]] = load ptr, ptr %[[FADDR1]]
; CHECK: %fa1 = getelementptr i64, ptr %[[LOAD1]], i64 %p1
; CHECK: %alloc_idx1 = call ptr @llvm.ptr.annotation.p0(ptr %fa1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %v1 = load i64, ptr %fa1
; CHECK: %alloc_idx2 = call ptr @llvm.ptr.annotation.p0(ptr %fa1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 0, ptr %fa1, align 8
; CHECK: %[[FADDR2:[0-9]+]] = getelementptr %__SOA_struct.test01, ptr @__soa_struct.test01, i64 0, i32 2
; CHECK: %[[LOAD2:[0-9]+]] = load ptr, ptr %[[FADDR2]]
; CHECK: %fa2 = getelementptr i32, ptr %[[LOAD2]], i64 %p1
; CHECK: %v2 = load i32, ptr %fa2

!intel.dtrans.types = !{!3, !4}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !1} ; { %struct.test01*, i32 }

