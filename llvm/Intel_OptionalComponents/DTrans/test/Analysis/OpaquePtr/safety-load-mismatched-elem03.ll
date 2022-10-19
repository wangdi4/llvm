; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases load a pointer type to access a scalar field.

; Using a pointer type to load a scalar field will always be treated as a
; 'Mismatched element access' and 'Bad casting'

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.pp8 = bitcast i32* %pField to i8**
  %vField = load i8*, i8** %pField.as.pp8

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i8* type.
  %use = load i8, i8* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.pp32 = bitcast i32* %pField to i32**
  %vField = load i32*, i32** %pField.as.pp32

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i32* type.
  %use = load i32, i32* %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.pp64 = bitcast i32* %pField to i64**
  %vField = load i64*, i64** %pField.as.pp64

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as an i64* type.
  %use = load i64, i64* %vField
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
%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32 }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test04a, %struct.test04a* %pStruct, i64 0, i32 1
  %pField.as.ppS4b = bitcast i32* %pField to %struct.test04b**
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
; CHECK: Safety data: Bad casting{{ *}}
; CHECK: End LLVMType: %struct.test04b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!13 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!14 = !{!"S", %struct.test04b zeroinitializer, i32 1, !1} ; { i32 }

!intel.dtrans.types = !{!10, !11, !12, !13, !14}
