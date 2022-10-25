; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-usecrulecompat -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test for indirect function calls which do not need to have the
; structure type passed marked as "Address taken" because there is
; no compatible type that could be operated upon by an address taken
; function.

%struct.test01a = type { i32, i32 }
@myarg = internal  global %struct.test01a { i32 3, i32 5 }
@fp = internal  global i32 (%struct.test01a*)* null, !intel_dtrans_type !4

define i32 @target1(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  %fieldAddr = getelementptr inbounds %struct.test01a, %struct.test01a* %pStruct, i32 0, i32 0
  %val = load i32, i32* %fieldAddr
  ret i32 %val
}

define i32 @main() {
  %fptr = load i32 (%struct.test01a*)*, i32 (%struct.test01a*)** @fp
  %res = call i32 %fptr(%struct.test01a* @myarg), !intel_dtrans_type !2
  ret i32 %res
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Global instance | Has initializer list{{ *$}}
; CHECK: End LLVMType: %struct.test01a


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !1, !3}  ; i32 (%struct.test01a*)
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{!2, i32 1}  ; i32 (%struct.test01a*)*
!5 = distinct !{!3}
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
