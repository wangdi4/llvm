; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -S -dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation of a simple type to check that
; a new type gets created by the transformation.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i64 }
; CHECK-OPAQUE: %__SOA_struct.test01 = type { ptr, ptr }

; CHECK-DAG: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
; CHECK-DAG: @__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
; CHECK-DAG: @__intel_dtrans_aostosoa_index = private constant [33 x i8] c"{dtrans} AOS-to-SOA index {id:0}\00"
; CHECK-DAG: @__intel_dtrans_aostosoa_filename = private constant [1 x i8] zeroinitializer

define i32 @main() {
  ret i32 0
}

!intel.dtrans.types = !{!3}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

; Verify the metadata was updated
; CHECK: !intel.dtrans.types = !{![[SMD:[0-9]+]]}

; CHECK: ![[SMD]] = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 2, ![[P32:[0-9]+]], ![[P64:[0-9]+]]}
; CHECK: ![[P32]] = !{i32 0, i32 1}
; CHECK: ![[P64]] = !{i64 0, i32 1}
