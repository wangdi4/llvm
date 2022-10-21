; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=false -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test AOS-to-SOA transformation update of the function metadata
; desciption for a function that takes a pointer-to-pointer of the
; type being transformed. In this case the metadata node for the
; parameter should be updated to indicate an integer pointer is
; passed in order for the downstream DTrans passes to have the
; correct pointer type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32 }
%struct.test01dep = type { %struct.test01*, %struct.test01* }

; Test a function that does not get cloned when opaque pointers are in use.
define "intel_dtrans_func_index"="1" %struct.test01dep* @test01(%struct.test01dep* "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !4 {
  ret %struct.test01dep* %in
}

; CHECK-NONOPAQUE: define internal "intel_dtrans_func_index"="1" %__SOADT_struct.test01dep* @test01.1(%__SOADT_struct.test01dep* "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type ![[FUNC1_MD:[0-9]+]]
; CHECK-OPAQUE: define "intel_dtrans_func_index"="1" ptr @test01(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type ![[FUNC1_MD:[0-9]+]]

; The return and parameter pointer-to-pointer should change to be a pointer to an integer type.
define "intel_dtrans_func_index"="1" %struct.test01** @test02(%struct.test01** "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !6 {
  ret %struct.test01** %in
}
; CHECK-NONOPAQUE: define internal "intel_dtrans_func_index"="1" i64* @test02.2(i64* "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type ![[FUNC2_MD:[0-9]+]]
; CHECK-OPAQUE: define "intel_dtrans_func_index"="1" ptr @test02(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type ![[FUNC2_MD:[0-9]+]]

; Test with dependent types, and multiple arguments.
define void @test03(%struct.test01** "intel_dtrans_func_index"="1" %in1, %struct.test01dep* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !7 {
  ret void
}
; CHECK-NONOPAQUE: define internal void @test03.3(i64* "intel_dtrans_func_index"="1" %in1, %__SOADT_struct.test01dep* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type ![[FUNC3_MD:[0-9]+]]
; CHECK-OPAQUE: define void @test03(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type ![[FUNC3_MD:[0-9]+]]

!intel.dtrans.types = !{!8, !9}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!4 = distinct !{!3, !3}
!5 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!6 = distinct !{!5, !5}
!7 = distinct !{!5, !3}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.test01*, i32 }
!9 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !2} ; { %struct.test01*, %struct.test01* }

; CHECK: !intel.dtrans.types = !{![[TEST01_MD:[0-9]+]], ![[DEP01_MD:[0-9]+]]}

; CHECK: ![[TEST01_MD]] = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 3, ![[PI32_MD:[0-9]+]], ![[PI64_MD:[0-9]+]], ![[PI32_MD]]}
; CHECK: ![[PI32_MD]] = !{i32 0, i32 1}
; CHECK: ![[PI64_MD]] = !{i64 0, i32 1}
; CHECK: ![[DEP01_MD]] = !{!"S", %__SOADT_struct.test01dep zeroinitializer, i32 2, ![[I64_MD:[0-9]+]], ![[I64_MD]]}
; CHECK: ![[I64_MD]] = !{i64 0, i32 0}
; CHECK: ![[FUNC1_MD]] = distinct !{![[PDEP_MD:[0-9]+]], ![[PDEP_MD]]}
; CHECK: ![[PDEP_MD]] = !{%__SOADT_struct.test01dep zeroinitializer, i32 1}
; CHECK: ![[FUNC2_MD]] = distinct !{![[PI64_MD]], ![[PI64_MD]]}
; CHECK: ![[FUNC3_MD]] = distinct !{![[PI64_MD]], ![[PDEP_MD:[0-9]+]]}
