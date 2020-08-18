; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases use scalar types for the fields and the loaded type.

; These cases should trigger the "mismatched element access" safety flag
; because the incorrect scalar type is used to access the field.

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %pStruct) !dtrans_type !2 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast i32* %pField to i8*
  %vField = load i8, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Mismatched element access{{ *$}}


%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* %pStruct) !dtrans_type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast i32* %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Mismatched element access{{ *$}}


%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* %pStruct) !dtrans_type !9 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32* %pField to i64*
  %pVal = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Mismatched element access{{ *$}}


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
!12 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!dtrans_types = !{!12, !13, !14}
