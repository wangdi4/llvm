; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a nested structure type, and the
; load type does not correspond to the first element of the nested type.

%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !3 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField.as.p8 = bitcast %struct.test01b* %pField to i8*
  %vField = load i8, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; Safety data: Mismatched element access | Nested structure{{ *$}}


%struct.test02a = type { %struct.test02b }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !8 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField.as.p16 = bitcast %struct.test02b* %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}


%struct.test03a = type { %struct.test03b }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct) !dtrans_type !12 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test03b* %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}


; Access that could be considered a whole structure reference, but we will
; not handle that in DTrans, so it should be marked as "Mismatched element
; access".
%struct.test04a = type { %struct.test04b }
%struct.test04b = type { i32, i32 }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !16 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test04b* %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}


; Access of 'i32' field within the nested structure as a different
; structure type is a "Mismatched element access". The cast structure
; type should be marked as "Bad casting", but not "Mismatched
; element access"
%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
%struct.test05c = type { i64 }
define void @test05(%struct.test05a* %pStruct) !dtrans_type !21 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.ppS5c = bitcast %struct.test05b* %pField to %struct.test05c**
  %vField = load %struct.test05c*, %struct.test05c** %pField.as.ppS5c
  %use = getelementptr %struct.test05c, %struct.test05c* %vField, i64 0, i32 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: Safety data: Bad casting | Mismatched element access | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05c
; CHECK: Safety data: Bad casting{{ *$}}


; A safe access to a nested element.
%struct.test06a = type { %struct.test06b }
%struct.test06b = type { i32, i32, i32 }
define void @test06(%struct.test06a* %pStruct) !dtrans_type !25 {
  %pField = getelementptr %struct.test06a, %struct.test06a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test06b* %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06a
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06b
; CHECK: Safety data: Nested structure{{ *$}}



!1 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!8 = !{!"F", i1 false, i32 1, !4, !9}  ; void (%struct.test02a*)
!9 = !{!10, i32 1}  ; %struct.test02a*
!10 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!11 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!12 = !{!"F", i1 false, i32 1, !4, !13}  ; void (%struct.test03a*)
!13 = !{!14, i32 1}  ; %struct.test03a*
!14 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!15 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!16 = !{!"F", i1 false, i32 1, !4, !17}  ; void (%struct.test04a*)
!17 = !{!18, i32 1}  ; %struct.test04a*
!18 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!19 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!20 = !{i64 0, i32 0}  ; i64
!21 = !{!"F", i1 false, i32 1, !4, !22}  ; void (%struct.test05a*)
!22 = !{!23, i32 1}  ; %struct.test05a*
!23 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!24 = !{!"R", %struct.test06b zeroinitializer, i32 0}  ; %struct.test06b
!25 = !{!"F", i1 false, i32 1, !4, !26}  ; void (%struct.test06a*)
!26 = !{!27, i32 1}  ; %struct.test06a*
!27 = !{!"R", %struct.test06a zeroinitializer, i32 0}  ; %struct.test06a
!28 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!29 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test02a zeroinitializer, i32 1, !7} ; { %struct.test02b }
!31 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!32 = !{!"S", %struct.test03a zeroinitializer, i32 1, !11} ; { %struct.test03b }
!33 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!34 = !{!"S", %struct.test04a zeroinitializer, i32 1, !15} ; { %struct.test04b }
!35 = !{!"S", %struct.test04b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!36 = !{!"S", %struct.test05a zeroinitializer, i32 1, !19} ; { %struct.test05b }
!37 = !{!"S", %struct.test05b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!38 = !{!"S", %struct.test05c zeroinitializer, i32 1, !20} ; { i64 }
!39 = !{!"S", %struct.test06a zeroinitializer, i32 1, !24} ; { %struct.test06b }
!40 = !{!"S", %struct.test06b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!dtrans_types = !{!28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40}
