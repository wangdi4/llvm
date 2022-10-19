; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test detection of "Unsafe pointer merge" safety condition when a type that
; is known to alias a pointer type has been converted to an integer and
; merged with a type that did not alias a pointer type.

%struct.test01 = type { i64, i64 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i64 %n) !intel.dtrans.func.type !3 {
  %tmp = ptrtoint %struct.test01* %pStruct to i64
  %sel = select i1 undef, i64 %tmp, i64 %n
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Unsafe pointer merge{{ *$}}


%struct.test02 = type { i64, i64 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %tmp = ptrtoint %struct.test02* %pStruct to i64
  %sel = select i1 undef, i64 %tmp, i64 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Unsafe pointer merge{{ *$}}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!7 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!6, !7}
