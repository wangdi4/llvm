; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base aren't
; mapped together since there is an access to the field that was reserved
; for padding using a byte-flattened GEP.

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define i8 @test01(%struct.test.a* %aptr) {
  %padcast = bitcast %struct.test.a* %aptr to i8*
  %agep = getelementptr i8, i8* %padcast, i64 8
  %ret = load i8, i8* %agep
  ret i8 %ret
}

; CHECK-LABEL: LLVMType: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a.base

; CHECK-LABEL: LLVMType: %struct.test.a.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a

%struct.test.a2 = type <{ i32, i32, [4 x i8] }>
%struct.test.a2.base = type <{ i32, i32 }>

define i8 @test02(%struct.test.a2* %aptr) {
  %padcast = bitcast %struct.test.a2* %aptr to i8*
  %agep = getelementptr i8, i8* %padcast, i64 8
  %agep2 = getelementptr i8, i8* %agep, i64 2
  %ret = load i8, i8* %agep2
  ret i8 %ret
}

; CHECK-LABEL: LLVMType: %struct.test.a2 = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a2.base

; CHECK-LABEL: LLVMType: %struct.test.a2.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a2