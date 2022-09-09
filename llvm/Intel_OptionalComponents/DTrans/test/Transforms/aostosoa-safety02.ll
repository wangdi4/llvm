; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that the dtrans aostosoa disqualified a structure that
; contains types that are not supported by the transformation.
; Currently a structure containing array elements is not supported.
%struct.test01 = type { i16, i16, [2 x i16] }

define void @test01() {
  %p = call i8* @malloc(i64 32)
  %p_test = bitcast i8* %p to %struct.test01*
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Unsupported structure element type: struct.test01

declare i8* @malloc(i64)

