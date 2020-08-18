; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a scalar type, and the load
; type is a pointer type.

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pStruct.as.pp8 = bitcast %struct.test01* %pStruct to i8**
  %vField = load i8*, i8** %pStruct.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i8* type.
  %use = load i8, i8* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %pStruct.as.pp32 = bitcast %struct.test02* %pStruct to i32**
  %vField = load i32*, i32** %pStruct.as.pp32

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i32* type.
  %use = load i32, i32* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %pStruct.as.pp64 = bitcast %struct.test03* %pStruct to i64**
  %vField = load i64*, i64** %pStruct.as.pp64

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i64* type.
  %use = load i64, i64* %vField
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}


%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32 }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !12 {
  %pStruct.as.ppS4b = bitcast %struct.test04a* %pStruct to %struct.test04b**
  %vField = load %struct.test04b*, %struct.test04b** %pStruct.as.ppS4b
  %use_test04 = getelementptr %struct.test04b, %struct.test04b* %vField, i64 0, i32 0
  ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; No elements of %struct.test04b are being accessed, so there should not be
; 'Mismatched element access' on it.
; CHECK: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
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
!15 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!18 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!19 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }

!dtrans_types = !{!15, !16, !17, !18, !19}
