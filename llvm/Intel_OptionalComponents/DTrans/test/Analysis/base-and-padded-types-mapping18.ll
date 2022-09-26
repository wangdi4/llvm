; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that accessing the zero element in a structure
; with base type doesn't breaks the relationship with the padded type.

; CHECK-LABEL: LLVMType: %class.inner2 = type <{ %class.inner3, [4 x i8] }>
; CHECK: Related base structure: class.inner2.base

; CHECK-LABEL: LLVMType: %class.inner2.base = type <{ %class.inner3 }>
; CHECK: Related padded structure: class.inner2

%class.outer = type <{ %class.inner1 }>
%class.inner1 = type <{ %class.inner2.base }>
%class.inner2.base = type <{ %class.inner3 }>
%class.inner3 = type <{ %class.TestClass*, %class.TestClass*, %class.TestClass* }>
%class.TestClass = type <{i64, i64, i64}>

%class.inner2 = type <{ %class.inner3, [4 x i8] }>

define internal void @foo(%class.outer* nocapture %arg, %class.inner2* %arg2) {
bb:
  %tmp = getelementptr %class.outer, %class.outer* %arg, i64 0, i32 0
  %tmp2 = bitcast %class.inner1* %tmp to i64*
  %tmp3 = load i64, i64* %tmp2

  ret void
}