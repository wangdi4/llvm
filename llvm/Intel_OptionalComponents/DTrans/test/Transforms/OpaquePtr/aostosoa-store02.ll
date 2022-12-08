; REQUIRES: asserts
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation of storing a null pointer to the type being
; transformed by AOS-to-SOA. When opaque pointers are used, there will only be a
; single constant value for 'null' within a function that can represent any
; pointer type. We want to make sure that only specific store instructions get
; changed to be integers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, ptr, i32 }
%struct.test01dep = type { ptr, ptr }

@var01 = internal global %struct.test01dep zeroinitializer
define i32 @test01() {
  %p0 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 0
  %p1 = getelementptr %struct.test01dep, ptr @var01, i64 0, i32 1

  ; This store should stay as storing a pointer null
  store ptr null, ptr %p0

  ; This store should be changed to store the integer zero
  store ptr null, ptr %p1
  ret i32 0
}
; CHECK-LABEL: define i32 @test01

; CHECK: %p0 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 0
; CHECK: %p1 = getelementptr %__SOADT_struct.test01dep, ptr @var01, i64 0, i32 1
; CHECK: store ptr null, ptr %p0
; CHECK: call ptr @llvm.ptr.annotation.p0(ptr %p1, ptr @__intel_dtrans_aostosoa_index, ptr @__intel_dtrans_aostosoa_filename, i32 0, ptr null)
; CHECK: store i64 0, ptr %p1
; CHECK: ret i32 0

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !3, !2} ; { %struct.test01dep*, %struct.test01* }

!intel.dtrans.types = !{!4, !5}
