; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test select statements that are safe.

; This case is safe because both arguments of the select are
; the same pointer type.
%struct.test01 = type { ptr, ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %pStruct.in) !intel.dtrans.func.type !2 {
  %pField0 = getelementptr %struct.test01, ptr %pStruct.in, i64 0, i32 0
  %pField1 = getelementptr %struct.test01, ptr %pStruct.in, i64 0, i32 1
  %pStruct0 = load ptr, ptr %pField0
  %pStruct1 = load ptr, ptr %pField1
  %chosen = select i1 undef, ptr %pStruct0, ptr %pStruct1
  call void @test01(ptr %chosen)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; This case is safe because the structure type can be safely aliased
; as a pointer to the element zero type.
%struct.test02 = type { ptr, ptr }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %pStruct.in, ptr "intel_dtrans_func_index"="2" %other) !intel.dtrans.func.type !6 {
  %chosen = select i1 undef, ptr %pStruct.in, ptr %other
  %val = load ptr, ptr %chosen

  ; This is needed because otherwise the pointer type analyzer cannot figure
  ; out a type for the bitcast result.
  %use = load i32, ptr %val
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


!1 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!2 = distinct !{!1}
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = !{i32 0, i32 2}  ; i32**
!6 = distinct !{!4, !5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { %struct.test01*, %struct.test01* }
!8 = !{!"S", %struct.test02 zeroinitializer, i32 2, !3, !4} ; { i32*, %struct.test02* }

!intel.dtrans.types = !{!7, !8}
