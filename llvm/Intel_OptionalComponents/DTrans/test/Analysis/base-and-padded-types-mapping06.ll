; REQUIRES: asserts

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-test-padded-structs=true -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-test-padded-structs=true -disable-output 2>&1 | FileCheck %s

; This test verifies that the padded field is marked as "Dirty" because
; %struct.test.a is marked as Address taken.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define i32 @test(%struct.test.a* %ptr) {
  %ret = call i32 @foo(%struct.test.a* %ptr)
  ret i32 %ret
}

declare i32 @foo(%struct.test.a* %ptr)

; CHECK-LABEL: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info:{{.*}}Dirty PaddedField{{.*}}
; CHECK: Safety data:{{.*}}Address taken{{.*}}

; CHECK-LABEL: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data:{{.*}}Address taken{{.*}}
