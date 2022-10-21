; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation update of the function metadata desciption
; for a function that takes a pointer-to-pointer of the type being transformed.
; This test is for cases where the function is cloned when opaque or non-opaque
; pointers are in use.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

; The return and parameter pointer to the structure will be converted to an integer index.
define "intel_dtrans_func_index"="1" %struct.test01* @test01(%struct.test01* "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !3 {
  ret %struct.test01* %in
}
; CHECK: define internal i64 @test01.1(i64 %in)
; CHECK-NOT: !intel.dtrans.func.type

; The return pointer to the structure should be converted to an integer index.
; The parameter pointer should be converted to a pointer to an integer type
define "intel_dtrans_func_index"="1" %struct.test01* @test02(%struct.test01** "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !5 {
  %p = load %struct.test01*, %struct.test01** %in
  ret %struct.test01* %p
}
; CHECK-NONOPAQUE: define internal i64 @test02.2(i64* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type ![[FUNC2_MD:[0-9]+]]
; CHECK-OPAQUE: define internal i64 @test02.2(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type ![[FUNC2_MD:[0-9]+]]

!intel.dtrans.types = !{!6, !7}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!5 = distinct !{!2, !4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!7 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

; CHECK: !intel.dtrans.types = !{![[TEST01_MD:[0-9]+]], ![[DEP01_MD:[0-9]+]]}

; CHECK: ![[TEST01_MD]] = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 3, ![[PI32_MD:[0-9]+]], ![[PI64_MD:[0-9]+]], ![[PI32_MD]]}
; CHECK: ![[PI32_MD]] = !{i32 0, i32 1}
; CHECK: ![[PI64_MD]] = !{i64 0, i32 1}
; CHECK: ![[DEP01_MD]] = !{!"S", %__SOADT_struct.test01dep zeroinitializer, i32 2, ![[I64_MD:[0-9]+]], ![[I64_MD]]}
; CHECK: ![[I64_MD]] = !{i64 0, i32 0}
; CHECK: ![[FUNC2_MD]] = distinct !{![[PI64_MD]]}
