; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This is a regression test for CMPLRLLVM-12195, in which dtransanalysis failed
; with an out of memory error due to not being able to process a GEP that did
; not contain the optional index values. This test checks that this special case
; does not crash the compiler.

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %in) {
  %tmp = getelementptr %struct.test01, %struct.test01* %in
  %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
  store i32 0, i32* %f0
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32 }
; CHECK: Safety data: No issues found
