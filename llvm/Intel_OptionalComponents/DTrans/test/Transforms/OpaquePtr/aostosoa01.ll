; RUN: opt -whole-program-assume -S -dtrans-aostosoaop %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -S -passes=dtrans-aostosoaop %s 2>&1 | FileCheck %s

; Test AOS-to-SOA transformation of a simple type to check that
; a new type gets created by the transformation.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i64 }

; TODO: Currently, this test is just checking that the pass can be run via opt.
; This test will be expanded to check that a new structure type gets created as
; more of the transformation is implemented.

; CHECK: %struct.test01 = type { i32, i64 }

define i32 @main() {
  ret i32 0
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

!intel.dtrans.types = !{!3}
