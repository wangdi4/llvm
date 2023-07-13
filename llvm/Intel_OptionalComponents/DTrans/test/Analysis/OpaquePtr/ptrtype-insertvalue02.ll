; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the PointerTypeAnalyzer does not mark 'InsertValue'
; instructions as 'Unhandled' when there are no pointers involved,
; as the types can be taken directly from the IR.

define void @test(i64 %in) {
  %iv1 = insertvalue { i64, i1 } { i64 poison, i1 false }, i64 %in, 0
; CHECK: %iv1 = insertvalue { i64, i1 } { i64 poison, i1 false }, i64 %in, 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK-NEXT: No aliased types

  ret void
}

!intel.dtrans.types = !{}
