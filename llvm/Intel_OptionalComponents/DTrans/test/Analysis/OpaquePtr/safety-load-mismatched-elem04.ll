; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer, but a different pointer
; type is used for the load.
;
; These cases should trigger the 'Mismatched element access' safety flag, unless
; the pointer is loaded using a generic pointer type.
; These also trigger 'Bad casting' because the field type within the structure
; is a pointer.

%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast i32** %pField to i8**
  %vField = load i8*, i8** %pField.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify %vField
  ; as being used as an i8* type. This load does not trigger any safety issues
  ; because it is accessing the members pointed to by the i32*, and not an
  ; aggregate type. This is consistent with the legacy LocalPointerAnalyzer.
  %use = load i8, i8* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.pp16 = bitcast i32** %pField to i16**
  %vField = load i16*, i16** %pField.as.pp16

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i16* type.
  %use = load i16, i16* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast i32** %pField to i64**
  %vField = load i64*, i64** %pField.as.pp64

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i64* type.
  %use = getelementptr i64, i64* %vField, i64 1
  %load = load i64, i64* %use
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


; In this case, the 'Bad casting' should also be marked on the structure
; type that was used for the loaded result, but 'Mismatched element access'
; should not be set on it because it was not accessing a field of that
; structure.
%struct.test04a = type { i32*, i32*, i32* }
%struct.test04b = type { i32* }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !12 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4b = bitcast i32** %pField to %struct.test04b**
  %vField = load %struct.test04b*, %struct.test04b** %pField.as.ppS4b
  %use_test04 = getelementptr %struct.test04b, %struct.test04b* %vField, i64 0, i32 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


%struct.test05 = type { i32**, i32** }
define void @test05(%struct.test05* %pStruct) !dtrans_type !16 {
  %pField = getelementptr %struct.test05, %struct.test05* %pStruct, i64 0, i32 1
  %pField.as.pp32 = bitcast i32*** %pField to i32**
  %vField = load i32*, i32** %pField.as.pp32
  %vUse = load i32, i32* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}



!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 1, !3, !7}  ; void (%struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 1, !3, !10}  ; void (%struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"F", i1 false, i32 1, !3, !13}  ; void (%struct.test04a*)
!13 = !{!14, i32 1}  ; %struct.test04a*
!14 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!15 = !{i32 0, i32 2}  ; i32**
!16 = !{!"F", i1 false, i32 1, !3, !17}  ; void (%struct.test05*)
!17 = !{!18, i32 1}  ; %struct.test05*
!18 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!19 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!20 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!22 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!23 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }
!24 = !{!"S", %struct.test05 zeroinitializer, i32 2, !15, !15} ; { i32**, i32** }

!dtrans_types = !{!19, !20, !21, !22, !23, !24}
