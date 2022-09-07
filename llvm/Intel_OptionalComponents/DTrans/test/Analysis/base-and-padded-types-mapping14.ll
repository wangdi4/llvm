; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base aren't
; mapped together since there is a memcpy that access incorrectly the field
; that was reserved for padding.

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"

%struct.test.a = type <{ i32, i32, [4 x i8] }>
%struct.test.a.base = type <{ i32, i32 }>

define void @test01(%struct.test.a* %a1, %struct.test.a.base* %a2) {
  %p1 = bitcast %struct.test.a* %a1 to i8*
  %p2 = bitcast %struct.test.a.base* %a2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 10, i1 false)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test.a = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a.base

; CHECK-LABEL: LLVMType: %struct.test.a.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a

; This test checks when there is a memcpy with padded structures but
; the whole structure is not copied.

%struct.test.a2 = type <{ i32, i32, [4 x i8] }>
%struct.test.a2.base = type <{ i32, i32 }>

define void @test02(%struct.test.a2* %a1, %struct.test.a2* %a2) {
  %p1 = bitcast %struct.test.a2* %a1 to i8*
  %p2 = bitcast %struct.test.a2* %a2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 10, i1 false)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test.a2 = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a2.base

; CHECK-LABEL: LLVMType: %struct.test.a2.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a2

; This test checks when there is a memcpy with padded structures but
; the size is not constant.

%struct.test.a3 = type <{ i32, i32, [4 x i8] }>
%struct.test.a3.base = type <{ i32, i32 }>

define void @test03(%struct.test.a3* %a1, %struct.test.a3* %a2, i64 %size) {
  %p1 = bitcast %struct.test.a3* %a1 to i8*
  %p2 = bitcast %struct.test.a3* %a2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 %size, i1 false)
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test.a3 = type <{ i32, i32, [4 x i8] }>
; CHECK-NOT: Related base structure: struct.test.a3.base

; CHECK-LABEL: LLVMType: %struct.test.a3.base = type <{ i32, i32 }>
; CHECK-NOT: Related padded structure: struct.test.a3

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)