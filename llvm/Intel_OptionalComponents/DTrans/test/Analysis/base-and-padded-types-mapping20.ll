; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-test-padded-structs=false -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-test-padded-structs=false -disable-output 2>&1 | FileCheck %s

; This test verifies that the structures %struct.test.a and
; %struct.test.a.base aren't marked as StructCouldHaveABIPadding and
; StructCouldBeBaseABIPadding respectively since there is a read
; for the field that is reserved for padding. Also, the related types
; should not be set.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define i8 @test(%struct.test.a* %ptr) {
  %arr = getelementptr %struct.test.a, %struct.test.a* %ptr, i64 0, i32 2
  %arrgep = getelementptr [4 x i8], [4 x i8]* %arr, i64 0, i32 0
  %ret = load i8, i8* %arrgep

  ret i8 %ret
}

declare i32 @foo(%struct.test.a* %ptr)

; Check that structure %struct.test.a is not set as ABI padding and
; the candidate field (field 2) is not set as PaddedField. Also,
; %struct.test.a.base should not be the related type.
; CHECK-LABEL: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK-NOT:    Field info: {{.*}}PaddedField{{.*}}
; CHECK-NOT: Safety data:{{.*}}Structure may have ABI padding{{.*}}

; Check that %struct.test.a.base is not set as base type and
; %struct.test.a is not the related type.
; CHECK-LABEL: %struct.test.a.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a
; CHECK-NOT: Safety data:{{.*}}Structure could be base for ABI padding{{.*}}
