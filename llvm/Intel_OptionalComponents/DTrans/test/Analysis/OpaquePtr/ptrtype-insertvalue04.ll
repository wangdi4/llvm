; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the PointerTypeAnalyzer does mark 'InsertValue'
; instructions as 'Unhandled' when the type does not have
; a pointer operand, but the source value is derived from a
; PtrToInt operator.
; 
%struct.foo = type { i32, i32 }

@foo = global %struct.foo zeroinitializer

define void @test(i64 %in) {
  %iv1 = insertvalue { i64, i1 } { i64 0, i1 false }, i64 ptrtoint (ptr @foo to i64), 0
; CHECK: %iv1 = insertvalue
; CHECK-NEXT: CE: i64 ptrtoint (ptr @foo to i64)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.foo*
; CHECK-NEXT: No element pointees.

; CHECK: LocalPointerInfo:
; CHECK-SAME: UNHANDLED

  ret void
; CHECK: ret void
}

!intel.dtrans.types = !{}
