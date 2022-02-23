; REQUIRES: asserts
; RUN: opt -S -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -S -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s 
; RUN: opt -opaque-pointers -S -whole-program-assume -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -S -whole-program-assume -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation is not triggered when not using
; Intel AVX2.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i64 }
; CHECK: %struct.test01 = type { i32, i64 }
; CHECK-NOT: %__SOA_struct.test01
; CHECK-NOT: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer

define i32 @main() {
  ret i32 0
}

!intel.dtrans.types = !{!3}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }
