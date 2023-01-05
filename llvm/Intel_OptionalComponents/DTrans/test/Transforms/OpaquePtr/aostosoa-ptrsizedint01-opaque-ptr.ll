; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This tests the AOS-to-SOA transform for handling loads/stores that
; are done as pointer sized int types instead of pointers of the type
; being transformed when using a 32-bit index for the peeling index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i64, ptr }

; Test with cast to pointer sized int for uses in load/store instructions
define internal void @test01(ptr "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !5 {
; CHECK-LABEL: define internal void @test01

  %field = getelementptr %struct.test01, ptr %st, i64 0, i32 1
  ; Use the result of the bitcast to load a value. Because the transformation
  ; is using a 32-bit value for the index, the load needs to be transformed to
  ; avoid exceeding the element size. Once the structure is transformed, %field
  ; will be an i32*, so that address should be used directly, rather than
  ; casting it to an i64* and back to an i32*. The result of the load may need
  ; to be extended to be compatible with the uses when the uses will stay as
  ; 64-bit operations. In this case, the icmp will use the 64-bit value, but
  ; the store needs to use the 32-bit result.
  %val_i64 = load i64, ptr %field
  %cmp = icmp eq i64 %val_i64, 0

; Verify a 32-bit load has been created, and extended to 64-bits.
; CHECK:  %[[LOADED:[0-9]+]] = load i32,
; CHECK:  %val_i64 = zext i32 %[[LOADED]] to i64

  ; Verify a 32-bit store has been created to write the index value.
  store i64 %val_i64, ptr %field
; CHECK: store i32 %[[LOADED]],

  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i8 0, i32 2}  ; i8**
!4 = distinct !{!3}
!5 = distinct !{!2}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!8}
