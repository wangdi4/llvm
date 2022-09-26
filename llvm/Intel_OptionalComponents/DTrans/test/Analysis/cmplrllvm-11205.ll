; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

; This is a regression test for CMPLRLLVM-11205 to verify that
; a compiler assertion is not triggered when the element pointee
; passed to dtrans::getParentStructType is an array type, rather
; than a structure type.

@test01str = private constant [10 x i8] c"123456789\00"

define void @test01(i64 %in) {
  %p = getelementptr [10 x i8], [10 x i8]* @test01str, i64  0, i64 %in
  %p1 = getelementptr i8, i8* %p, i64 -1
  call void @test01user(i8* %p1)
  ret void
}

define void @test01user(i8* %in) {
  %c = load i8, i8* %in
  ret void
}

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [10 x i8]
