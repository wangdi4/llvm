; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a pointer type that does not match the field's
; pointer type. In these cases, the pointer operand of the store instruction
; is cast to a different type.


%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %value = call i8* @malloc(i64 4)
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast i32** %pField to i8**
  store i8* %value, i8** %pField.as.pp8
  ret void
}
; Memory allocation gets inferred based on the type it is used as.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


%struct.test01a = type { i32*, i32*, i64* }
define void @test01a(%struct.test01a* %pStruct) !dtrans_type !7 {
  %value = call i8* @malloc(i64 4)
  %pField1 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField2 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 2
  %pField1.as.pp8 = bitcast i32** %pField1 to i8**
  store i8* %value, i8** %pField1.as.pp8
  %pField2.as.pp8 = bitcast i64** %pField2 to i8**
  store i8* %value, i8** %pField2.as.pp8
  ret void
}
; Here the memory allocation gets inferred as two different types, resulting in
; a safety condition.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct, i16* %value) !dtrans_type !10 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.pp16 = bitcast i32** %pField to i16**
  store i16* %value, i16** %pField.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* %pStruct, i64* %value) !dtrans_type !14 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast i32** %pField to i64**
  store i64* %value, i64** %pField.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test04a = type { i32*, i32*, i32* }
%struct.test04b = type { i32* }
define void @test04(%struct.test04a* %pStruct, %struct.test04b* %value) !dtrans_type !17 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4b = bitcast i32** %pField to %struct.test04b**
  store %struct.test04b* %value, %struct.test04b** %pField.as.ppS4b
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

declare i8* @malloc(i64)


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{i64 0, i32 1}  ; i64*
!7 = !{!"F", i1 false, i32 1, !3, !8}  ; void (%struct.test01a*)
!8 = !{!9, i32 1}  ; %struct.test01a*
!9 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!10 = !{!"F", i1 false, i32 2, !3, !11, !13}  ; void (%struct.test02*, i16*)
!11 = !{!12, i32 1}  ; %struct.test02*
!12 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!13 = !{i16 0, i32 1}  ; i16*
!14 = !{!"F", i1 false, i32 2, !3, !15, !6}  ; void (%struct.test03*, i64*)
!15 = !{!16, i32 1}  ; %struct.test03*
!16 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!17 = !{!"F", i1 false, i32 2, !3, !18, !20}  ; void (%struct.test04a*, %struct.test04b*)
!18 = !{!19, i32 1}  ; %struct.test04a*
!19 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!20 = !{!21, i32 1}  ; %struct.test04b*
!21 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!22 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!23 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !6} ; { i32*, i32*, i64* }
!24 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!25 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!26 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!27 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }

!dtrans_types = !{!22, !23, !24, !25, !26, !27}
