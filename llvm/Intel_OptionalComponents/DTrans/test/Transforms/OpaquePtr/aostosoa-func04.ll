; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s 

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation with a function that returns the
; type being transformed from a pointer to an integer when the
; return value is a 'null' pointer.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, ptr }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p1 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %res = call ptr @test02()
  store  ptr %res, ptr %p1
  ret i32 0
}
; CHECK-LABEL: define i32 @test01

; CHECK: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK: %res = call i64 @test02.1()
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 %res, ptr %p1, align 8
; CHECK: ret i32 0

define "intel_dtrans_func_index"="1" ptr @test02()  !intel.dtrans.func.type !3 {
  %p1 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1
  %v1 = load ptr, ptr %p1
  ret ptr null
}
; CHECK-LABEL: define internal i64 @test02

; CHECK: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK: %alloc_idx = call ptr @llvm.ptr.annotation.p0.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: %v1 = load i64, ptr %p1, align 8
; CHECK: ret i64 0


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
