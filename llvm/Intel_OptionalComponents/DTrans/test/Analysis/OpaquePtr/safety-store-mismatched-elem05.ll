; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a pointer type that does not match the field's
; pointer type. In these cases, the value operand of the store instruction
; is cast to a different type.


%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %value = call i8* @malloc(i64 4)
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %value.as.p32 = bitcast i8* %value to i32*
  store i32* %value.as.p32, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* %pStruct, i16* %value) !dtrans_type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %value.as.p32 = bitcast i16* %value to i32*
  store i32* %value.as.p32, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* %pStruct, i64* %value) !dtrans_type !10 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %value.as.p32 = bitcast i64* %value to i32*
  store i32* %value.as.p32, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test04a = type { i32*, i32*, i32* }
%struct.test04b = type { i32* }
define void @test04(%struct.test04a* %pStruct, %struct.test04b* %value) !dtrans_type !14 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %value.as.p32 = bitcast %struct.test04b* %value to i32*
  store i32* %value.as.p32, i32** %pField
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
!6 = !{!"F", i1 false, i32 2, !3, !7, !9}  ; void (%struct.test02*, i16*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{i16 0, i32 1}  ; i16*
!10 = !{!"F", i1 false, i32 2, !3, !11, !13}  ; void (%struct.test03*, i64*)
!11 = !{!12, i32 1}  ; %struct.test03*
!12 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!13 = !{i64 0, i32 1}  ; i64*
!14 = !{!"F", i1 false, i32 2, !3, !15, !17}  ; void (%struct.test04a*, %struct.test04b*)
!15 = !{!16, i32 1}  ; %struct.test04a*
!16 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!17 = !{!18, i32 1}  ; %struct.test04b*
!18 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!19 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!20 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!22 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!23 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }

!dtrans_types = !{!19, !20, !21, !22, !23}
