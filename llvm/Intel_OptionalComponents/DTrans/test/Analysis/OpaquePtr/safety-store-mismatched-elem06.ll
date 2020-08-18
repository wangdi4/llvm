; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a scalar type where a pointer to a structure type is
; expected in order to trigger an 'unsafe pointer store' safety violation, on
; the stored type.

%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct, i8 %value) !dtrans_type !4 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast %struct.test01b** %pField to i8*
  store i8 %value, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


%struct.test02a = type { %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct, i16 %value) !dtrans_type !11 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast %struct.test02b** %pField to i16*
  store i16 %value, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


; This case is unsafe even though it is using a pointer sized int for the store
; because it is unknown whether the integer value actually represents the right
; kind of pointer.
%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct, i64 %value) !dtrans_type !17 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast %struct.test03b** %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


; This case is safe for using a pointer-sized integer for the store because
; the 'ptrtoint' instruction result will be associated with the expected
; pointer type.
%struct.test04a = type { %struct.test04b*, %struct.test04b*, %struct.test04b* }
%struct.test04b = type { i32, i32, i32 }
define void @test04(%struct.test04a* %pStructA, %struct.test04b* %pStructB) !dtrans_type !23 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStructA, i64 0, i32 1
  %pField.as.p64 = bitcast %struct.test04b** %pField to i64*
  %pStructB.as.i64 = ptrtoint %struct.test04b* %pStructB to i64
  store i64 %pStructB.as.i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: No issues found


!1 = !{!2, i32 1}  ; %struct.test01b*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 2, !5, !6, !8}  ; void (%struct.test01a*, i8)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01a*
!7 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!8 = !{i8 0, i32 0}  ; i8
!9 = !{!10, i32 1}  ; %struct.test02b*
!10 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!11 = !{!"F", i1 false, i32 2, !5, !12, !14}  ; void (%struct.test02a*, i16)
!12 = !{!13, i32 1}  ; %struct.test02a*
!13 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!14 = !{i16 0, i32 0}  ; i16
!15 = !{!16, i32 1}  ; %struct.test03b*
!16 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!17 = !{!"F", i1 false, i32 2, !5, !18, !20}  ; void (%struct.test03a*, i64)
!18 = !{!19, i32 1}  ; %struct.test03a*
!19 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!20 = !{i64 0, i32 0}  ; i64
!21 = !{!22, i32 1}  ; %struct.test04b*
!22 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!23 = !{!"F", i1 false, i32 2, !5, !24, !21}  ; void (%struct.test04a*, %struct.test04b*)
!24 = !{!25, i32 1}  ; %struct.test04a*
!25 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!26 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!27 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!28 = !{!"S", %struct.test02a zeroinitializer, i32 3, !9, !9, !9} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!29 = !{!"S", %struct.test02b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test03a zeroinitializer, i32 3, !15, !15, !15} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!31 = !{!"S", %struct.test03b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!32 = !{!"S", %struct.test04a zeroinitializer, i32 3, !21, !21, !21} ; { %struct.test04b*, %struct.test04b*, %struct.test04b* }
!33 = !{!"S", %struct.test04b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }

!dtrans_types = !{!26, !27, !28, !29, !30, !31, !32, !33}
