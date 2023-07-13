; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on load instructions


; This is a special case of a load that uses 'undef' as its pointer operand.
define internal void @test01() {
  %var_pi64 = load ptr, ptr undef
  %var_i64 = load i64, ptr %var_pi64
  ret void
}
; CHECK-LABEL: void @test01
; CHECK: %var_pi64 = load ptr, ptr undef
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: No element pointees.



!intel.dtrans.types = !{}
