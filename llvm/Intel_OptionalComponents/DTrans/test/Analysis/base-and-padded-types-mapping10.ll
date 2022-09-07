; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base aren't
; mapped together since there is an access to the field that was reserved
; for padding. This case takes care when there is a zero element access
; using a bitcast.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define i8 @test(%struct.test.b* %bptr) {
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 2
  %padgep = getelementptr [4 x i8], [4 x i8]* %agep, i64 0, i64 0
  %ret = load i8, i8* %padgep
  ret i8 %ret
}

; CHECK-LABEL: LLVMType: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a.base

; CHECK-LABEL: LLVMType: %struct.test.a.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a
