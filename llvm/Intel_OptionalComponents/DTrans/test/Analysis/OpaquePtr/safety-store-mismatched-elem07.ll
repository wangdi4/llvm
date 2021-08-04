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
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
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
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct, i16* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !8 {
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
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct, i64* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !12 {
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
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct, %struct.test04c* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !17 {
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


!1 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = !{i16 0, i32 1}  ; i16*
!8 = distinct !{!6, !7}
!9 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!10 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!11 = !{i64 0, i32 1}  ; i64*
!12 = distinct !{!10, !11}
!13 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!14 = !{i16 0, i32 0}  ; i16
!15 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!16 = !{%struct.test04c zeroinitializer, i32 1}  ; %struct.test04c*
!17 = distinct !{!15, !16}
!18 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!19 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!20 = !{!"S", %struct.test02a zeroinitializer, i32 3, !5, !5, !5} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!21 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!22 = !{!"S", %struct.test03a zeroinitializer, i32 3, !9, !9, !9} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!23 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!24 = !{!"S", %struct.test04a zeroinitializer, i32 3, !13, !13, !13} ; { %struct.test04b*, %struct.test04b*, %struct.test04b* }
!25 = !{!"S", %struct.test04b zeroinitializer, i32 3, !2, !2, !2} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test04c zeroinitializer, i32 2, !14, !14} ; { i16, i16 }

!intel.dtrans.types = !{!18, !19, !20, !21, !22, !23, !24, !25, !26}
