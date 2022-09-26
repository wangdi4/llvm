; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-test-padded-structs=true -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-test-padded-structs=true -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base are
; mapped together since there is a memcpy that access correctly the field
; that was reserved for padding.

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define void @test01(%struct.test.a* %a1, %struct.test.a.base* %a2) {
  %p1 = bitcast %struct.test.a* %a1 to i8*
  %p2 = bitcast %struct.test.a.base* %a2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 8, i1 false)
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

; CHECK-LABEL: LLVMType: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK: Related base structure: struct.test.a.base

; CHECK-LABEL: LLVMType: %struct.test.a.base = type <{ i32, i32 }>
; CHECK: Related padded structure: struct.test.a