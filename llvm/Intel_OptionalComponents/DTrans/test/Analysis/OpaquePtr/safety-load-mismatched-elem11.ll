; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to the start of a structure, but
; loads a type that does not match the type of the first element of the
; structure.

; These cases are for when the field element is a pointer to a structure type,
; and the load type is a scalar type.

%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !4 {
  %pStruct.as.p8 = bitcast %struct.test01a* %pStruct to i8*
  %vField = load i8, i8* %pStruct.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting{{ *$}}


%struct.test02a = type { %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !10 {
  %pStruct.as.p16 = bitcast %struct.test02a* %pStruct to i16*
  %vField = load i16, i16* %pStruct.as.p16
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
  %pStruct.as.p64 = bitcast %struct.test03a* %pStruct to i64*
  %vField = load i64, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: No issues found


%struct.test04a = type { %struct.test04b*, %struct.test04b*, %struct.test04b* }
%struct.test04b = type { i32, i32, i32 }
%struct.test04c = type { i16, i16 }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !21 {
  %pStruct.as.ppS4c = bitcast %struct.test04a* %pStruct to %struct.test04c**
  %vField = load %struct.test04c*, %struct.test04c** %pStruct.as.ppS4c
  %use4b = getelementptr %struct.test04c, %struct.test04c* %vField, i64 0, i32 1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04c
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}


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
!18 = !{!19, i32 1}  ; %struct.test04b*
!19 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!20 = !{i16 0, i32 0}  ; i16
!21 = !{!"F", i1 false, i32 1, !5, !22}  ; void (%struct.test04a*)
!22 = !{!23, i32 1}  ; %struct.test04a*
!23 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!24 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!25 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test02a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!27 = !{!"S", %struct.test02b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!28 = !{!"S", %struct.test03a zeroinitializer, i32 3, !13, !13, !13} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!29 = !{!"S", %struct.test03b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test04a zeroinitializer, i32 3, !18, !18, !18} ; { %struct.test04b*, %struct.test04b*, %struct.test04b* }
!31 = !{!"S", %struct.test04b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!32 = !{!"S", %struct.test04c zeroinitializer, i32 2, !20, !20} ; { i16, i16 }

!dtrans_types = !{!24, !25, !26, !27, !28, !29, !30, !31, !32}
