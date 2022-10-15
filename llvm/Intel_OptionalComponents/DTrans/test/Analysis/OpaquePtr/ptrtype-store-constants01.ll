; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the PointerTypeAnalyzer class does not crash, when a store
; instruction uses compiler constants for the memory locations of the
; value and pointer operands.

define void @test() {
  store ptr poison, ptr null
  ret void
}

!intel.dtrans.types = !{}

; CHECK: define void @test()
