; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on simple load instructions


; Simple test of load that results in an i64*
@var01 = internal global ptr null, !intel_dtrans_type !1
define internal void @test01() {
  %var_pi64 = load ptr, ptr @var01
  %var_i64 = load i64, ptr %var_pi64
  ret void
}
; CHECK-LABEL: void @test01
; CHECK: %var_pi64 = load ptr, ptr @var01
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: No element pointees.

!1 = !{i64 0, i32 1}  ; i64*

!intel.dtrans.types = !{}
