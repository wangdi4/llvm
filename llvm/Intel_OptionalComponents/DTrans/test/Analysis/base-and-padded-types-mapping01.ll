; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base are mapped together
; since they have a base-padded types relationship. Also, check that the last field
; in %struct.test.a is marked as the padded field.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define i32 @test(%struct.test.b* %bptr) {
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 0
  %ret = load i32, i32* %agep
  ret i32 %ret
}

; CHECK-LABEL: LLVMType: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info: PaddedField

; CHECK-LABEL: LLVMType: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a
