; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a stored location is a field in a structure, and the
; stored type does not match the field type.
; These cases are storing a pointer type that does not match the field's
; pointer type. In these cases, the pointer operand of the store instruction
; is cast to a different type.


%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %value = call i8* @malloc(i64 4)
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast i32** %pField to i8**
  store i8* %value, i8** %pField.as.pp8
  ret void
}
; Memory allocation gets inferred based on the type it is used as.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found


%struct.test01a = type { i32*, i32*, i64* }
define void @test01a(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %value = call i8* @malloc(i64 4)
  %pField1 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 1
  %pField2 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 2
  %pField1.as.pp8 = bitcast i32** %pField1 to i8**
  store i8* %value, i8** %pField1.as.pp8
  %pField2.as.pp8 = bitcast i64** %pField2 to i8**
  store i8* %value, i8** %pField2.as.pp8
  ret void
}
; Here the memory allocation gets inferred as two different types, resulting in
; a safety condition.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01

%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct, i16* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.pp16 = bitcast i32** %pField to i16**
  store i16* %value, i16** %pField.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct, i64* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !11 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast i32** %pField to i64**
  store i64* %value, i64** %pField.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


%struct.test04a = type { i32*, i32*, i32* }
%struct.test04b = type { i32* }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct, %struct.test04b* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !14 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4b = bitcast i32** %pField to %struct.test04b**
  store %struct.test04b* %value, %struct.test04b** %pField.as.ppS4b
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}
; CHECK: End LLVMType: %struct.test04b

declare !intel.dtrans.func.type !16 "intel_dtrans_func_index"="1" i8* @malloc(i64)


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!8 = !{i16 0, i32 1}  ; i16*
!9 = distinct !{!7, !8}
!10 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!11 = distinct !{!10, !4}
!12 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!13 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!14 = distinct !{!12, !13}
!15 = !{i8 0, i32 1}  ; i8*
!16 = distinct !{!15}
!17 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!18 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !4} ; { i32*, i32*, i64* }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!20 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!22 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }

!intel.dtrans.types = !{!17, !18, !19, !20, !21, !22}
