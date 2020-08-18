; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test select statements that are safe.

; This case is safe because both arguments of the select are
; the same pointer type.
%struct.test01 = type { %struct.test01*, %struct.test01* }
define internal void @test01(%struct.test01* %pStruct.in) !dtrans_type !3 {
  %pField0 = getelementptr %struct.test01, %struct.test01* %pStruct.in, i64 0, i32 0
  %pField1 = getelementptr %struct.test01, %struct.test01* %pStruct.in, i64 0, i32 1
  %pStruct0 = load %struct.test01*, %struct.test01** %pField0
  %pStruct1 = load %struct.test01*, %struct.test01** %pField1
  %chosen = select i1 undef, %struct.test01* %pStruct0, %struct.test01* %pStruct1
  call void @test01(%struct.test01* %chosen)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


; This case is safe because the structure type can be safely aliased
; as a pointer to the element zero type.
%struct.test02 = type { i32*, %struct.test02* }
define internal void @test02(%struct.test02* %pStruct.in, i32** %other) !dtrans_type !8 {
  %pStruct.in.elemzero = bitcast %struct.test02* %pStruct.in to i32**
  %chosen = select i1 undef, i32** %pStruct.in.elemzero, i32** %other
  %val = load i32*, i32** %chosen

  ; This is needed because otherwise the pointer type analyzer cannot figure
  ; out a type for the bitcast result.
  %use = load i32, i32* %val
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: No issues found


!1 = !{!2, i32 1}  ; %struct.test01*
!2 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!3 = !{!"F", i1 false, i32 1, !4, !1}  ; void (%struct.test01*)
!4 = !{!"void", i32 0}  ; void
!5 = !{i32 0, i32 1}  ; i32*
!6 = !{!7, i32 1}  ; %struct.test02*
!7 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!8 = !{!"F", i1 false, i32 2, !4, !6, !9}  ; void (%struct.test02*, i32**)
!9 = !{i32 0, i32 2}  ; i32**
!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { %struct.test01*, %struct.test01* }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !6} ; { i32*, %struct.test02* }

!dtrans_types = !{!10, !11}
