; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on simple load instructions

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Simple test of load that results in an i64*
@var01 = internal global i64* null, !dtrans_type !1
define internal void @test01() {
  %var_pi64 = load i64*, i64** @var01
  %var_i64 = load i64, i64* %var_pi64
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR: %var_pi64 = load i64*, i64** @var01
; CHECK-FUT: %var_pi64 = load p0, p0 @var01
; CHECK:      Aliased types:
; CHECK:        i64*
; CHECK:      No element pointees.

!1 = !{i64 0, i32 1}  ; i64*

!dtrans_types = !{}
