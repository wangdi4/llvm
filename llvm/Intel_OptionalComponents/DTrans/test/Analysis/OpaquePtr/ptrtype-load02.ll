; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on load instructions

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; This is a special case of a load that uses 'undef' as its pointer operand.
define internal void @test01() {
  %var_pi64 = load i64*, i64** undef
  %var_i64 = load i64, i64* %var_pi64
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR: %var_pi64 = load i64*, i64** undef
; CHECK-FUT: %var_pi64 = load p0, p0 undef
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: No element pointees.


!dtrans_types = !{}

