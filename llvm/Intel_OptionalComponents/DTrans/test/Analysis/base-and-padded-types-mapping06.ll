; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-test-padded-structs=true -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-test-padded-structs=true -disable-output 2>&1 | FileCheck %s

; This test verifies that the padded field is marked as "Dirty" because
; the padded field in %struct.test.a is read and -dtrans-test-padded-structs
; is enabled. The goal of this test is to check if the option
; -dtrans-test-padded-structs is working for debugging purposes.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define i8 @test(%struct.test.a* %ptr) {
  %arr = getelementptr %struct.test.a, %struct.test.a* %ptr, i64 0, i32 2
  %arrgep = getelementptr [4 x i8], [4 x i8]* %arr, i64 0, i32 0
  %ret = load i8, i8* %arrgep

  ret i8 %ret
}

declare i32 @foo(%struct.test.a* %ptr)

; Check that %struct.test.a is masked as padded structure but the padded
; field is marked as dirty.
; CHECK-LABEL: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK-NEXT: Field info:{{.*}}Dirty PaddedField{{.*}}
; CHECK: Safety data:{{.*}}Structure may have ABI padding{{.*}}

; Structure %struct.test.a.base should be marked as base structure.
; CHECK-LABEL: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data:{{.*}}Structure could be base for ABI padding{{.*}}
