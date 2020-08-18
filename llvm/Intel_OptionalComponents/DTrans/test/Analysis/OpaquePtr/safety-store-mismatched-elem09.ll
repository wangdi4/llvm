; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing an incorrect scalar type to a location that holds the
; address of a nested structure member.


%struct.test01a = type { %struct.test01b }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct, i8 %value) !dtrans_type !3 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField.as.p8 = bitcast %struct.test01b* %pField to i8*
  store i8 %value, i8* %pField.as.p8
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
define void @test02(%struct.test02a* %pStruct, i16 %value) !dtrans_type !9 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField.as.p16 = bitcast %struct.test02b* %pField to i16*
  store i16 %value, i16* %pField.as.p16
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
define void @test03(%struct.test03a* %pStruct, i64 %value) !dtrans_type !14 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test03b* %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}


; This access could be considered a whole structure reference, but we do not
; handle this case in DTrans for simplicity, so it should be marked as
; "mismatched element access".
%struct.test04a = type { %struct.test04b }
%struct.test04b = type { i32, i32 }
define void @test04(%struct.test04a* %pStruct, i64 %value) !dtrans_type !19 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 0
  %pField.as.p64 = bitcast %struct.test04b* %pField to i64*
  store i64 %value, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Mismatched element access | Nested structure{{ *$}}


; Access of 'i32' field within the nested structure as a different
; structure type.
%struct.test05a = type { %struct.test05b }
%struct.test05b = type { i32, i32 }
%struct.test05c = type { i64 }
define void @test05(%struct.test05a* %pStruct, %struct.test05c* %pStruct5c) !dtrans_type !23 {
  %pField = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField.as.ppS5c = bitcast %struct.test05b* %pField to %struct.test05c**
  store %struct.test05c* %pStruct5c, %struct.test05c** %pField.as.ppS5c
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05a
; CHECK: Safety data: Bad casting | Mismatched element access | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05b
; CHECK: Safety data: Bad casting | Mismatched element access | Unsafe pointer store | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05c
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; A safe access to a nested element.
%struct.test06a = type { %struct.test06b }
%struct.test06b = type { i32, i32, i32 }
define void @test06(%struct.test06a* %pStruct, i32 %value) !dtrans_type !29 {
  %pField = getelementptr %struct.test06a, %struct.test06a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test06b* %pField to i32*
  store i32 %value, i32* %pField.as.p32
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
!3 = !{!"F", i1 false, i32 2, !4, !5, !7}  ; void (%struct.test01a*, i8)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{i8 0, i32 0}  ; i8
!8 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!9 = !{!"F", i1 false, i32 2, !4, !10, !12}  ; void (%struct.test02a*, i16)
!10 = !{!11, i32 1}  ; %struct.test02a*
!11 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!12 = !{i16 0, i32 0}  ; i16
!13 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!14 = !{!"F", i1 false, i32 2, !4, !15, !17}  ; void (%struct.test03a*, i64)
!15 = !{!16, i32 1}  ; %struct.test03a*
!16 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!17 = !{i64 0, i32 0}  ; i64
!18 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!19 = !{!"F", i1 false, i32 2, !4, !20, !17}  ; void (%struct.test04a*, i64)
!20 = !{!21, i32 1}  ; %struct.test04a*
!21 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!22 = !{!"R", %struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!23 = !{!"F", i1 false, i32 2, !4, !24, !26}  ; void (%struct.test05a*, %struct.test05c*)
!24 = !{!25, i32 1}  ; %struct.test05a*
!25 = !{!"R", %struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!26 = !{!27, i32 1}  ; %struct.test05c*
!27 = !{!"R", %struct.test05c zeroinitializer, i32 0}  ; %struct.test05c
!28 = !{!"R", %struct.test06b zeroinitializer, i32 0}  ; %struct.test06b
!29 = !{!"F", i1 false, i32 2, !4, !30, !2}  ; void (%struct.test06a*, i32)
!30 = !{!31, i32 1}  ; %struct.test06a*
!31 = !{!"R", %struct.test06a zeroinitializer, i32 0}  ; %struct.test06a
!32 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!33 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!34 = !{!"S", %struct.test02a zeroinitializer, i32 1, !8} ; { %struct.test02b }
!35 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!36 = !{!"S", %struct.test03a zeroinitializer, i32 1, !13} ; { %struct.test03b }
!37 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!38 = !{!"S", %struct.test04a zeroinitializer, i32 1, !18} ; { %struct.test04b }
!39 = !{!"S", %struct.test04b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!40 = !{!"S", %struct.test05a zeroinitializer, i32 1, !22} ; { %struct.test05b }
!41 = !{!"S", %struct.test05b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!42 = !{!"S", %struct.test05c zeroinitializer, i32 1, !17} ; { i64 }
!43 = !{!"S", %struct.test06a zeroinitializer, i32 1, !28} ; { %struct.test06b }
!44 = !{!"S", %struct.test06b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }

!dtrans_types = !{!32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44}
