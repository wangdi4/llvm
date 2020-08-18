; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test detection of "Unsafe pointer merge" safety condition when a type that
; is known to alias a pointer type has been converted to an integer and
; merged with a type that did not alias a pointer type.

%struct.test01 = type { i64, i64 }
define void @test01(%struct.test01* %pStruct, i64 %n) !dtrans_type !2 {
  %tmp = ptrtoint %struct.test01* %pStruct to i64
  %sel = select i1 undef, i64 %tmp, i64 %n
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Unsafe pointer merge{{ *$}}


%struct.test02 = type { i64, i64 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %tmp = ptrtoint %struct.test02* %pStruct to i64
  %sel = select i1 undef, i64 %tmp, i64 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Unsafe pointer merge{{ *$}}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"F", i1 false, i32 2, !3, !4, !1}  ; void (%struct.test01*, i64)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!dtrans_types = !{!9, !10}
