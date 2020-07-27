; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a pointer to a different type than expected for a
; field that is a pointer to a structure. In these cases, the pointer operand
; of the store instruction has been cast to a different type.

%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !4 {
  %value = alloca i8
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast %struct.test01b** %pField to i8**
  store i8* %value, i8** %pField.as.pp8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


%struct.test02a = type{ %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct, i16* %value) !dtrans_type !10 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  %pField.as.pp16 = bitcast %struct.test02b** %pField to i16**
  store i16* %value, i16** %pField.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


%struct.test03a = type{ %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct, i64* %value) !dtrans_type !16 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast %struct.test03b** %pField to i64**
  store i64* %value, i64** %pField.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

%struct.test04a = type { %struct.test04b*, %struct.test04b*, %struct.test04b* }
%struct.test04b = type { i32, i32, i32 }
%struct.test04c = type { i16, i16 }
define void @test04(%struct.test04a* %pStruct, %struct.test04c* %value) !dtrans_type !23 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4c = bitcast %struct.test04b** %pField to %struct.test04c**
  store %struct.test04c* %value, %struct.test04c** %pField.as.ppS4c
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04c
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


!1 = !{!2, i32 1}  ; %struct.test01b*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01a*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01a*
!7 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!8 = !{!9, i32 1}  ; %struct.test02b*
!9 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!10 = !{!"F", i1 false, i32 2, !5, !11, !13}  ; void (%struct.test02a*, i16*)
!11 = !{!12, i32 1}  ; %struct.test02a*
!12 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!13 = !{i16 0, i32 1}  ; i16*
!14 = !{!15, i32 1}  ; %struct.test03b*
!15 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!16 = !{!"F", i1 false, i32 2, !5, !17, !19}  ; void (%struct.test03a*, i64*)
!17 = !{!18, i32 1}  ; %struct.test03a*
!18 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!19 = !{i64 0, i32 1}  ; i64*
!20 = !{!21, i32 1}  ; %struct.test04b*
!21 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!22 = !{i16 0, i32 0}  ; i16
!23 = !{!"F", i1 false, i32 2, !5, !24, !26}  ; void (%struct.test04a*, %struct.test04c*)
!24 = !{!25, i32 1}  ; %struct.test04a*
!25 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!26 = !{!27, i32 1}  ; %struct.test04c*
!27 = !{!"R", %struct.test04c zeroinitializer, i32 0}  ; %struct.test04c
!28 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!29 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test02a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!31 = !{!"S", %struct.test02b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!32 = !{!"S", %struct.test03a zeroinitializer, i32 3, !14, !14, !14} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!33 = !{!"S", %struct.test03b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!34 = !{!"S", %struct.test04a zeroinitializer, i32 3, !20, !20, !20} ; { %struct.test04b*, %struct.test04b*, %struct.test04b* }
!35 = !{!"S", %struct.test04b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!36 = !{!"S", %struct.test04c zeroinitializer, i32 2, !22, !22} ; { i16, i16 }

!dtrans_types = !{!28, !29, !30, !31, !32, !33, !34, !35, !36}
