; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-aostosoaop -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-aostosoaop -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-aostosoaop -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -debug-only=dtrans-aostosoaop -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that a safety condition on a dependent type will disable the
; AOS-to-SOA transformation on a type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32* }
%struct.test01dep = type { %struct.test01*, i32* }

define i32 @main() {
  %mem = call i8* @calloc(i64 1, i64 16)
  %dep_mem = bitcast i8* %mem to %struct.test01dep*

  ; Introduce the "Address Taken" safety flag on the dependent type. (Note, this
  ; also gets propagated to the type we are trying to convert, but this test is
  ; using the '-dtrans-aostosoaop-qual-override=true' flag to bypass that check)
  call void @unknown_func(%struct.test01dep* %dep_mem)
  ret i32 0
}

; CHECK: AOS-to-SOA disqualifying type: %struct.test01
; CHECK-SAME: based on safety conditions of dependent type: %struct.test01dep

declare !intel.dtrans.func.type !5 void @unknown_func(%struct.test01dep* "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!intel.dtrans.types = !{!8, !9}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!5 = distinct !{!4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.test01*, i32* }
!9 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !3} ; { %struct.test01*, i32* }
