; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test the PtrTypeAnalyzer handling of a load instruction that uses an inttoptr
; constant expression as the pointer operand that is not from a variable.


define internal i32 @test01() {
  %res = load i32, ptr inttoptr (i64 120 to ptr)
  ret i32 %res
}

; CHECK-LABEL: define internal i32 @test01
; CHECK: CE: ptr inttoptr (i64 120 to ptr)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: No element pointees.

!intel.dtrans.types = !{}


