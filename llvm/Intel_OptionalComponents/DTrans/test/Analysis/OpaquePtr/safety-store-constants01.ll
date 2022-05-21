; RUN: opt -opaque-pointers -S -whole-program-assume -passes='require<dtrans-safetyanalyzer>' %s 2>&1 | FileCheck %s

; Test that the DTransSafetyAnalyzer class does not crash, when a store
; instruction uses compiler constants for the memory locations of the
; value and pointer operands.

define void @test() {
  store ptr poison, ptr null
  ret void
}

!intel.dtrans.types = !{}

; CHECK: define void @test()
