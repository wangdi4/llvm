; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that DTrans analysis works even if there is an instruction
; with broken SSA form but it is in a dead block. LLVM allows to generate
; broken SSA instructions in dead blocks. This can produce an issue if they
; aren't handled properly.

; CHECK: DTRANS Analysis Types Created
; CHECK:  LLVMType: %class.TestClass = type { i64, i64, i64 }

%class.TestClass = type {i64, i64, i64}

define internal i64 @getElem(%class.TestClass* nocapture %arg) {
bb:
  %tmp = getelementptr %class.TestClass, %class.TestClass* %arg, i64 0, i32 0
  %tmp2 = load i64, i64* %tmp
  br label %bb3

bbDead:
  %tmpDead = getelementptr %class.TestClass, %class.TestClass* %tmpDead, i64 0
  br label %bbDead

bb3:
  ret i64 %tmp2
}
