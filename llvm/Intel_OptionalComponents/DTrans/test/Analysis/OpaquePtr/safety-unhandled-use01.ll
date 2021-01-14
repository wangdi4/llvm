; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that instructions not explicitly modeled in the DTransSafetyAnalyzer
; result in the "Unhandled use" safety flag.

; The 'trunc' instruction is not modeled because there are no known ways that it
; can produce a safe result for DTrans when the operand represents a pointer
; value. This should result in the structure type being marked as "Unhandled
; use".
%struct.test01 = type { i32, i32 }
define i32 @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %full = ptrtoint %struct.test01* %pStruct to i64
  %low = trunc i64 %full to i32
  ret i32 %low
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Unhandled use{{ *}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (%struct.test01*)
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!5}
