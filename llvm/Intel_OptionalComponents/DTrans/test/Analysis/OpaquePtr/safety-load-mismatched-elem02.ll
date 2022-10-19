; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases load a scalar type to access a pointer field.

; Using a scalar i8 type to access a pointer field is a 'mismatched element
; access'. This also triggers 'bad casting' because the structure field
; is not accessed by a compatible pointer type.
%struct.test01 = type { i32*, i32*, i32* }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  %pField.as.p8 = bitcast i32** %pField to i8*
  %vField = load i8, i8* %pField.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Using a scalar i16 type to access a pointer field is a 'mismatched element
; access'.This also triggers 'bad casting' because the structure field
; is not accessed by a compatible pointer type.
%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  %pField.as.p16 = bitcast i32** %pField to i16*
  %vField = load i16, i16* %pField.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


; This case does not trigger "Mismatched element access" because it is using
; a pointer sized int to load a pointer, which is treated as being safe, even
; though the type does not match the field type. If the result loaded
; subsequently gets used in an incompatible way that will trigger a Safety
; violation.
%struct.test03 = type { i32*, i32*, i32* }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 1
  %pField.as.p64 = bitcast i32** %pField to i64*
  %vField = load i64, i64* %pField.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


; Loading a pointer-to-pointer type as a scalar is a 'mismatched element
; access'.This also triggers 'bad casting' because the structure field
; is not accessed by a compatible pointer type.
%struct.test04 = type { i32**, i32** }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %pField = getelementptr %struct.test04, %struct.test04* %pStruct, i64 0, i32 1
  %pField.as.p32 = bitcast i32*** %pField to i32*
  %vField = load i32, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test04


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{i32 0, i32 2}  ; i32**
!9 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!10 = distinct !{!9}
!11 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!12 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!13 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32*, i32*, i32* }
!14 = !{!"S", %struct.test04 zeroinitializer, i32 2, !8, !8} ; { i32**, i32** }

!intel.dtrans.types = !{!11, !12, !13, !14}
