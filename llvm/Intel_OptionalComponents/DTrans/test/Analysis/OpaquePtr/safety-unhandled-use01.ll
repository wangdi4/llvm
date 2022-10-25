; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that instructions not explicitly modeled in the DTransSafetyAnalyzer
; result in the "Unhandled use" safety flag.

; The 'trunc' instruction is not modeled because there are no known ways that it
; can produce a safe result for DTrans when the operand represents a pointer
; value. This should result in the structure type being marked as "Unhandled
; use".
%struct.test01 = type { i32, i32 }
define i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !3 {
  %full = ptrtoint %struct.test01* %pStruct to i64
  %low = trunc i64 %full to i32
  ret i32 %low
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Unhandled use{{ *}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!4}
