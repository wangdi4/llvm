; REQUIRES: asserts
; RUN: opt -S -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation of binary operators used to determine
; the number of structure instances between two pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, %struct.test01*, i64 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

@var01 = internal global %struct.test01dep zeroinitializer
define i64 @test01() {
; CHECK-LABEL: define i64 @test01

  %p0 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 0
  %p1 = getelementptr %struct.test01dep, %struct.test01dep* @var01, i64 0, i32 1
  %o0 = load %struct.test01*, %struct.test01** %p0
  %o1 = load %struct.test01*, %struct.test01** %p1

  ; Calculate the distance between these two pointers.
  %o0.int = ptrtoint %struct.test01* %o0 to i64
  %o1.int = ptrtoint %struct.test01* %o1 to i64
  %byte_count = sub i64 %o0.int, %o1.int
  %num = sdiv i64 %byte_count, 24

; All ptrtoint instructions should have been removed
; CHECK-NOT: = ptrtoint
; CHECK: %byte_count = sub i64 %o0, %o1
; CHECK: %num = sdiv i64 %byte_count, 1

  ; Test with multiple of the structure size
  %numx3 = sdiv i64 %byte_count, 72
  %num2 = sdiv i64 %numx3, 3
; CHECK: %numx3 = sdiv i64 %byte_count, 3

  ret i64 %num
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i64, %struct.test01*, i64 }
!4 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

!intel.dtrans.types = !{!3, !4}
