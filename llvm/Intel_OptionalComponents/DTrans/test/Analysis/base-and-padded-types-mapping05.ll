; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that the base and padded types, %struct.test.a and
; %struct.test.a.base are marked as StructCouldHaveABIPadding and
; StructCouldBeBaseABIPadding respectively. Also, the related base/padded
; should be set correctly.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define i32 @test(%struct.test.a* %ptr) {
  %ret = call i32 @foo(%struct.test.a* %ptr)
  ret i32 %ret
}

declare i32 @foo(%struct.test.a* %ptr)

; %struct.test.a may have ABI padding and the related base structure is
; %struct.test.a.base
; CHECK-LABEL: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base
; CHECK:  2)Field LLVM Type: [4 x i8]
; CHECK-NEXT:    Field info: {{.*}}PaddedField{{.*}}
; CHECK: Safety data:{{.*}}Structure may have ABI padding{{.*}}

; %struct.test.a could be base for ABI padding and the related base structure
; is %struct.test.a
; CHECK-LABEL: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data:{{.*}}Structure could be base for ABI padding{{.*}}
