; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing into a pointer field type using a scalar type.

%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* %pStruct, i8 %value) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast i32** %pField to i8*
  store i8 %value, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct, i16 %value) !dtrans_type !7 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast i32** %pField to i16*
  store i16 %value, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


; This case triggers a "Mismatched element access" even though it is using
; a pointer sized int to store a pointer because there is no information about
; what the integer may actually represent.
%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* %pStruct, i64 %value) !dtrans_type !11 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32** %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


; This case does not trigger a "Mismatched element access" because it is using a
; pointer sized int to store a value that is known to be a pointer of the correct
; type.
%struct.test04 = type { i32*, i32*, i32* }
define void @test04(%struct.test04* %pStruct, i32* %value) !dtrans_type !15 {
  %pField = getelementptr %struct.test04, %struct.test04* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32** %pField to i64*
  %pValue.as.i64 = ptrtoint i32* %value to i64
  store i64 %pValue.as.i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"F", i1 false, i32 2, !3, !4, !6}  ; void (%struct.test01*, i8)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i8 0, i32 0}  ; i8
!7 = !{!"F", i1 false, i32 2, !3, !8, !10}  ; void (%struct.test02*, i16)
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{i16 0, i32 0}  ; i16
!11 = !{!"F", i1 false, i32 2, !3, !12, !14}  ; void (%struct.test03*, i64)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{i64 0, i32 0}  ; i64
!15 = !{!"F", i1 false, i32 2, !3, !16, !1}  ; void (%struct.test04*, i32*)
!16 = !{!17, i32 1}  ; %struct.test04*
!17 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!18 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!20 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }

!dtrans_types = !{!18, !19, !20, !21}
