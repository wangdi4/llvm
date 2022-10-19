; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify field value collection of structure fields results in
; the 'incomplete' value list condition for cases where a safety condition
; occurs.

%struct.test01 = type { i32, i32 }
define void @test01() {
  %local = alloca %struct.test01
  %pA = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 0
  ; Only write one field, to verify the other is not left as 'No value'
  store i32 1, i32* %pA

  ; Introduce an 'address taken' safety condition.
  call void @unknownFunc(%struct.test01* %local)
  ret void
}
declare !intel.dtrans.func.type !3 void @unknownFunc(%struct.test01* "intel_dtrans_func_index"="1")

; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 1 ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ ] <incomplete>
; CHECK:  Safety data: Address taken | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!4}
