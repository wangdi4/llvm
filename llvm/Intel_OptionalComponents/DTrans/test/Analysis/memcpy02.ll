; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memcpy that involve an element pointer
; for one of the pointers, when the other pointer is not an element pointee.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memcpy where the source and target types match, but the source
; pointer is a field within another structure, while the destination pointer is
; not.
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a }
define void @test01(%struct.test01a* %pStructA, %struct.test01b* %pStructB) {
  %pDst = bitcast %struct.test01a* %pStructA to i8*
  %pField = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 1
  %pSrc = bitcast %struct.test01a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
 ;CHECK: Safety data: Nested structure

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *}}
; CHECK: 1)Field LLVM Type: %struct.test01a
; CHECK: Field info: ComplexUse
; CHECK: Safety data: Contains nested structure


; Test with memcpy where the source and target types match, but the destination
; pointer is a field within another structure, while the source pointer is not.
%struct.test02a = type { i32, i32, i32, i32, i32 }
%struct.test02b = type { i32, %struct.test02a }
define void @test02(%struct.test02a* %pStructA, %struct.test02b* %pStructB) {
  %pSrc = bitcast %struct.test02a* %pStructA to i8*
  %pField = getelementptr %struct.test02b, %struct.test02b* %pStructB, i64 0, i32 1
  %pDst = bitcast %struct.test02a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
 ;CHECK: Safety data: Nested structure

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *}}
; CHECK: 1)Field LLVM Type: %struct.test02a
; CHECK: Field info: ComplexUse
; CHECK: Safety data: Contains nested structure

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

