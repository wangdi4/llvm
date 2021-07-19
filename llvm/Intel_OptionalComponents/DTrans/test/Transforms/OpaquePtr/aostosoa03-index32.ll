; REQUIRES: asserts
; RUN: opt -whole-program-assume -S -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -whole-program-assume -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -force-opaque-pointers -whole-program-assume -S -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -force-opaque-pointers -whole-program-assume -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test AOS-to-SOA transformation with a type that is referenced by another type.
; In this case, the type with the reference will need to be considered as a
; dependent type because a pointer within it will be converted to be an integer
; type to be used as an index for the structure of arrays.
;
; This is a variant of the test aostosoa03.ll, updated to use:
; -dtrans-aostosoaop-index32=true

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32* }
%struct.test01dep = type { %struct.test01*, i32* }

; CHECK-NONOPAQUE-DAG: %__SOA_struct.test01 = type { i32*, i32*, i32** }
; CHECK-NONOPAQUE-DAG: %__SOADT_struct.test01dep = type { i32, i32* }

; CHECK-OPAQUE-DAG: %__SOA_struct.test01 = type { ptr, ptr, ptr }
; CHECK-OPAQUE-DAG: %__SOADT_struct.test01dep = type { i32, ptr }

; CHECK-DAG: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
; CHECK-DAG: @__intel_dtrans_aostosoa_alloc = private constant [38 x i8] c"{dtrans} AOS-to-SOA allocation {id:0}\00"
; CHECK-DAG: @__intel_dtrans_aostosoa_index = private constant [33 x i8] c"{dtrans} AOS-to-SOA index {id:0}\00"
; CHECK-DAG: @__intel_dtrans_aostosoa_filename = private constant [1 x i8] zeroinitializer

define i32 @main() {
  ret i32 0
}

!intel.dtrans.types = !{!4, !5}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.test01*, i32 }
!5 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !3} ; { %struct.test01*, i32 }

; CHECK: !intel.dtrans.types = !{![[SMD1:[0-9]+]], ![[SMD2:[0-9]+]]}

; CHECK: ![[SMD1]] = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 3, ![[P32:[0-9]+]], ![[P32]], ![[PP32:[0-9]+]]}
; CHECK: ![[P32]] = !{i32 0, i32 1}
; CHECK: ![[PP32]] = !{i32 0, i32 2}

; CHECK: ![[SMD2]] = !{!"S", %__SOADT_struct.test01dep zeroinitializer, i32 2, ![[I32:[0-9]+]], ![[P32]]}
; CHECK: ![[I32]] = !{i32 0, i32 0}
