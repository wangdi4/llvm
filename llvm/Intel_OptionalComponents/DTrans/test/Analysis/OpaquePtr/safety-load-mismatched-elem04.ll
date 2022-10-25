; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer, but a different pointer
; type is used for the load.
;
; These cases should trigger the 'Mismatched element access' safety flag, unless
; the pointer is loaded using a generic pointer type.
; These also trigger 'Bad casting' because the field type within the structure
; is a pointer.

%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast i32** %pField to i8**
  %vField = load i8*, i8** %pField.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify %vField
  ; as being used as an i8* type. This load does not trigger any safety issues
  ; because it is accessing the members pointed to by the i32*, and not an
  ; aggregate type. This is consistent with the legacy LocalPointerAnalyzer.
  %use = load i8, i8* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.pp16 = bitcast i32** %pField to i16**
  %vField = load i16*, i16** %pField.as.pp16

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i16* type.
  %use = load i16, i16* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast i32** %pField to i64**
  %vField = load i64*, i64** %pField.as.pp64

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i64* type.
  %use = getelementptr i64, i64* %vField, i64 1
  %load = load i64, i64* %use
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test03


; In this case, the 'Bad casting' should also be marked on the structure
; type that was used for the loaded result, but 'Mismatched element access'
; should not be set on it because it was not accessing a field of that
; structure.
%struct.test04a = type { i32*, i32*, i32* }
%struct.test04b = type { i32* }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4b = bitcast i32** %pField to %struct.test04b**
  %vField = load %struct.test04b*, %struct.test04b** %pField.as.ppS4b
  %use_test04 = getelementptr %struct.test04b, %struct.test04b* %vField, i64 0, i32 0
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test04b


%struct.test05 = type { i32**, i32** }
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !12 {
  %pField = getelementptr %struct.test05, %struct.test05* %pStruct, i64 0, i32 1
  %pField.as.pp32 = bitcast i32*** %pField to i32**
  %vField = load i32*, i32** %pField.as.pp32
  %vUse = load i32, i32* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test05



!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!9 = distinct !{!8}
!10 = !{i32 0, i32 2}  ; i32**
!11 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!16 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!17 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32* }
!18 = !{!"S", %struct.test05 zeroinitializer, i32 2, !10, !10} ; { i32**, i32** }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
