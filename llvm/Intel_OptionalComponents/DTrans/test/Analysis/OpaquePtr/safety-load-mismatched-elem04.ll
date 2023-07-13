; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a load uses a pointer to a field in a structure, and the loaded
; type does not correspond to the field type.
; These cases are for when the field type is a pointer, but a different pointer
; type is used for the load.
;
; These cases should trigger the 'Mismatched element access' safety flag, unless
; the pointer is loaded using a generic pointer type.
; These also trigger 'Bad casting' because the field type within the structure
; is a pointer.

%struct.test01 = type { ptr, ptr, ptr }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; This instruction is needed for the pointer type analyzer to identify %vField
  ; as being used as an i8* type. This load does not trigger any safety issues
  ; because it is accessing the members pointed to by the i32*, and not an
  ; aggregate type. This is consistent with the legacy LocalPointerAnalyzer.
  %use = load i8, ptr %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { ptr, ptr, ptr }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %pField = getelementptr %struct.test02, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i16* type.
  %use = load i16, ptr %vField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.test02


%struct.test03 = type { ptr, ptr, ptr }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField = getelementptr %struct.test03, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField

  ; This instruction is needed for the pointer type analyzer to identify
  ; %vField as being used as a i64* type.
  %use = getelementptr i64, ptr %vField, i64 1
  %load = load i64, ptr %use
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
%struct.test04a = type { ptr, ptr, ptr }
%struct.test04b = type { ptr }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test04a, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField
  %use_test04 = getelementptr %struct.test04b, ptr %vField, i64 0, i32 0
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


%struct.test05 = type { ptr, ptr }
define void @test05(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !12 {
  %pField = getelementptr %struct.test05, ptr %pStruct, i64 0, i32 1
  %vField = load ptr, ptr %pField
  %vUse = load i32, ptr %vField
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
