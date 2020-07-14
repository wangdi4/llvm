; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer to an aggregate type,
; but gets accessed as a scalar type.


%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !4 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast %struct.test01b** %pField to i8*
  %vField = load i8, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting


%struct.test02a = type { %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !10 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast %struct.test02b** %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad casting{{ *$}}


; This case does not trigger a "Mismatched element access" because it is using
; a pointer sized int to load a pointer.
%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct) !dtrans_type !15 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast %struct.test03b** %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: No issues found


%struct.test04a = type { %struct.test04b** }
%struct.test04b = type { i32, i32, i32 }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !20 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 0
  %pField.as.p16 = bitcast %struct.test04b*** %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting{{ *$}}


; This case does not trigger a "Mismatched element access" because it is using
; a pointer sized int to load a pointer to a pointer. Note, %vField is still
; being tracked as a %struct.test05b**, so could result in a safety bit later
; based on its uses.
%struct.test05a = type { %struct.test05b**, %struct.test05b*, %struct.test05b* }
%struct.test05b = type { i32, i32, i32 }
define void @test05(%struct.test05a* %pStruct) !dtrans_type !26 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test05b*** %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: Safety data: No issues found

!1 = !{!2, i32 1}  ; %struct.test01b*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01a*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01a*
!7 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!8 = !{!9, i32 1}  ; %struct.test02b*
!9 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!10 = !{!"F", i1 false, i32 1, !5, !11}  ; void (%struct.test02a*)
!11 = !{!12, i32 1}  ; %struct.test02a*
!12 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!13 = !{!14, i32 1}  ; %struct.test03b*
!14 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!15 = !{!"F", i1 false, i32 1, !5, !16}  ; void (%struct.test03a*)
!16 = !{!17, i32 1}  ; %struct.test03a*
!17 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!18 = !{!19, i32 2}  ; %struct.test04b**
!19 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!20 = !{!"F", i1 false, i32 1, !5, !21}  ; void (%struct.test04a*)
!21 = !{!22, i32 1}  ; %struct.test04a*
!22 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!23 = !{!24, i32 2}  ; %struct.test05b**
!24 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!25 = !{!24, i32 1}  ; %struct.test05b*
!26 = !{!"F", i1 false, i32 1, !5, !27}  ; void (%struct.test05a*)
!27 = !{!28, i32 1}  ; %struct.test05a*
!28 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!29 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!30 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!31 = !{!"S", %struct.test02a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!32 = !{!"S", %struct.test02b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!33 = !{!"S", %struct.test03a zeroinitializer, i32 3, !13, !13, !13} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!34 = !{!"S", %struct.test03b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!35 = !{!"S", %struct.test04a zeroinitializer, i32 1, !18} ; { %struct.test04b** }
!36 = !{!"S", %struct.test04b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!37 = !{!"S", %struct.test05a zeroinitializer, i32 3, !23, !25, !25} ; { %struct.test05b**, %struct.test05b*, %struct.test05b* }
!38 = !{!"S", %struct.test05b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }

!dtrans_types = !{!29, !30, !31, !32, !33, !34, !35, !36, !37, !38}
