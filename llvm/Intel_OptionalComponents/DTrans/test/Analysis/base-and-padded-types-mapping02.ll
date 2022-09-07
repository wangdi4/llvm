; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that bad casting will not show up in the safety
; data since we are casting from a base type to a padded type. Also,
; it checks that %struct.test.a is related to %struct.test.a.base.
; The safety data results from %struct.test.a and %struct.test.a.base
; should be the same.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define i32 @test(%struct.test.b* %bptr) {
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 0
  %ret = load i32, i32* %agep
  ret i32 %ret
}

; CHECK-LABEL: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}

; CHECK-LABEL: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}

; CHECK-LABEL: %struct.test.b = type { %struct.test.a.base, i64 }
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}
